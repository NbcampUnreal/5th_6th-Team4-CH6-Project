#include "Server/UKGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "Systems/UK_GameInstance.h"

AUKGameMode::AUKGameMode()
{
	// stub
}

void AUKGameMode::InitGame(
	const FString& MapName,
	const FString& Options,
	FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());

	if ( GI && GI->CharacterSelected )
	{
		DefaultPawnClass = GI->CharacterSelected;
	}
}

void AUKGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 데이터 테이블 확인
	if (!MonsterRewardTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] MonsterRewardTable is NULL!"));
		UE_LOG(LogTemp, Error, TEXT("Please assign DT_MonsterRewards in GameMode Blueprint!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] MonsterRewardTable assigned: %s"), 
			*MonsterRewardTable->GetName());
	}

	// 맵에 이미 배치된 모든 몬스터 등록
	TArray<AActor*> FoundMonsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIMonsterBase::StaticClass(), FoundMonsters);

	int32 RegisteredCount = 0;
	for (AActor* Actor : FoundMonsters)
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Actor))
		{
			RegisterMonster(Monster);
			RegisteredCount++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Registered %d monsters from map"), RegisteredCount);
}

void AUKGameMode::RegisterMonster(AAIMonsterBase* Monster)
{
	if (!Monster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] RegisterMonster: Monster is NULL!"));
		return;
	}

	// OnMonsterKilled 델리게이트 바인딩
	// 중복 바인딩 방지
	Monster->OnMonsterKilled.RemoveAll(this);
	Monster->OnMonsterKilled.AddDynamic(this, &AUKGameMode::OnMonsterKilled);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Registered: %s (Type=%d)"),
		*Monster->GetName(), (int32)Monster->MonsterType);
}

void AUKGameMode::OnMonsterSpawned(AAIMonsterBase* Monster)
{
	if (!Monster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] OnMonsterSpawned: Monster is NULL!"));
		return;
	}

	// 새로 스폰된 몬스터 등록
	RegisterMonster(Monster);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] New monster spawned and registered: %s (Type=%d)"),
		*Monster->GetName(), (int32)Monster->MonsterType);
}

void AUKGameMode::OnMonsterKilled(
	AAIMonsterBase* KilledMonster,
	EMonsterType MonsterType,
	APlayerController* KillerController)
{
	if (!KillerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Monster killed but no killer found!"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("[GameMode] ========== MONSTER KILLED ========== "));
	UE_LOG(LogTemp, Error, TEXT("Monster: %s (Type=%d)"), 
		*KilledMonster->GetName(), (int32)MonsterType);
	UE_LOG(LogTemp, Error, TEXT("Killer: %s"), 
		*KillerController->GetName());

	// 보상 지급
	GiveRewardToPlayer(KillerController, MonsterType);

	UE_LOG(LogTemp, Error, TEXT("================================================"));
}

FName AUKGameMode::GetRowNameFromMonsterType(EMonsterType MonsterType)
{
	// EMonsterType → Row Name 변환
	switch (MonsterType)
	{
	case EMonsterType::Golem:    
		return FName("Golem");
	default:                     
		UE_LOG(LogTemp, Error, TEXT("[GameMode] Unknown MonsterType: %d"), (int32)MonsterType);
		return FName("None");
	}
}

void AUKGameMode::GiveRewardToPlayer(
	APlayerController* PlayerController,
	EMonsterType MonsterType)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("[Reward] PlayerController is NULL!"));
		return;
	}

	// 데이터 테이블 체크
	if (!MonsterRewardTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[Reward] MonsterRewardTable is NULL!"));
		UE_LOG(LogTemp, Error, TEXT("Please assign DT_MonsterRewards in GameMode!"));
		return;
	}

	// MonsterType → Row Name 변환
	FName RowName = GetRowNameFromMonsterType(MonsterType);
	
	if (RowName == FName("None"))
	{
		UE_LOG(LogTemp, Error, TEXT("[Reward] Invalid MonsterType!"));
		return;
	}

	// 데이터 테이블에서 보상 정보 가져오기
	static const FString ContextString(TEXT("Monster Reward Lookup"));
	FUK_ItemData* RewardData = MonsterRewardTable->FindRow<FUK_ItemData>(
		RowName, 
		ContextString
	);

	if (!RewardData)
	{
		UE_LOG(LogTemp, Error, TEXT("[Reward] No reward data found for Row: %s"), 
			*RowName.ToString());
		UE_LOG(LogTemp, Error, TEXT("Check if DT_MonsterRewards has a row named '%s'"), 
			*RowName.ToString());
		return;
	}

	// 보상 정보 로그
	//UE_LOG(LogTemp, Error, TEXT(" [Reward] Found data for: %s"), *RewardData->MonsterName);
	//UE_LOG(LogTemp, Error, TEXT("  Gold: %d"), RewardData->Gold);
	//UE_LOG(LogTemp, Error, TEXT("  Exp: %d"), RewardData->Exp);
	//UE_LOG(LogTemp, Error, TEXT("  Item Count: %d"), RewardData->ItemDrops.Num());

	// 플레이어 Pawn 가져오기
	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] Player has no pawn!"));
		return;
	}

	/*
	 
	// 캐릭터에게 보상 지급
	if (AUK_CharacterBase* Character = Cast<AUK_CharacterBase>(PlayerPawn))
	{
		// ===== 골드 지급 =====
		if (RewardData->Gold > 0)
		{
			// 실제 골드 지급 함수 호출
			// Character->AddGold(RewardData->Gold);
			
			UE_LOG(LogTemp, Warning, TEXT("[Reward] Gave %d Gold to %s"), 
				RewardData->Gold, *PlayerController->GetName());
		}

		// ===== 경험치 지급 =====
		if (RewardData->Exp > 0)
		{
			// 실제 경험치 지급 함수 호출
			// Character->AddExp(RewardData->Exp);
			
			UE_LOG(LogTemp, Warning, TEXT("[Reward] Gave %d Exp to %s"), 
				RewardData->Exp, *PlayerController->GetName());
		}

		// ===== 아이템 드랍 (확률 기반) =====
		for (int32 i = 0; i < RewardData->ItemDrops.Num(); i++)
		{
			// 드랍 확률 가져오기 (기본 100%)
			float DropChance = 1.0f;
			if (RewardData->DropChances.IsValidIndex(i))
			{
				DropChance = RewardData->DropChances[i];
			}

			// 확률 체크
			float RandomValue = FMath::FRand(); // 0.0 ~ 1.0
			
			if (RandomValue <= DropChance)
			{
				FName ItemID = RewardData->ItemDrops[i];
				
				// Character->AddItem(ItemID);
				
				UE_LOG(LogTemp, Warning, TEXT("[Reward] Dropped item: %s (chance: %.1f%%, roll: %.1f%%)"),
					*ItemID.ToString(), DropChance * 100.0f, RandomValue * 100.0f);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[Reward] Item %s NOT dropped (chance: %.1f%%, roll: %.1f%%)"),
					*RewardData->ItemDrops[i].ToString(), DropChance * 100.0f, RandomValue * 100.0f);
			}
		}

		UE_LOG(LogTemp, Error, TEXT("[Reward] All rewards processed successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Reward] PlayerPawn is not UK_CharacterBase!"));
		UE_LOG(LogTemp, Error, TEXT("Pawn class: %s"), *PlayerPawn->GetClass()->GetName());
	}
	
	*/
}