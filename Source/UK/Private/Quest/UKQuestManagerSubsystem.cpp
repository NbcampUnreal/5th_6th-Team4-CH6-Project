#include "Quest/UKQuestManagerSubsystem.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "server/UKSaveGame.h"

bool UUKQuestManagerSubsystem::StartQuest(FName QuestId)
{
	if ( QuestId.IsNone() ) return false;
	if ( RuntimeProgress.Contains(QuestId) ) return true;

	RuntimeProgress.Add(QuestId, FQuestProgress{});
	return true;
}

bool UUKQuestManagerSubsystem::CompleteQuest(FName QuestId)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->bCompleted = true;
	//추가: 보상 지급 및 해금 로직

	// 1. 구체적인 퀘스트별 보상 처리 (예시: 허수아비, 골렘 퀘스트)
	if (QuestId == "Quest_Scarecrow")
	{
		// 경험치 지급(100);
		UE_LOG(LogTemp, Log, TEXT("허수아비 퀘스트 완료: 경험치 획득!"));
		
		
	}
	else if (QuestId == "Quest_Golem")
	{
		// 아이템 보상
		GiveQuestReward(TEXT("Item_Stone"), 5);
		GiveQuestReward(TEXT("Item_GolemCore"), 1);

		// 경험치 지급 (HUD나 캐릭터 시스템에 신호)
		// 경험치 지급(500)
		UE_LOG(LogTemp, Log, TEXT("골렘 퀘스트 완료: Get Stone and GolemCore!"));
	}
	
	return true;
}

bool UUKQuestManagerSubsystem::SetQuestStep(FName QuestId, int32 NewStep)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->Step = NewStep;
	return true;
}

void UUKQuestManagerSubsystem::EmitQuestEvent(FName EventId)
{
	// TODO: 나중에 DA/JSON과 연결해서 이벤트->진행도 갱신 로직을 여기서 수행
	// 지금은 뼈대만 둡니다.
}

bool UUKQuestManagerSubsystem::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
	UUKSaveGame* SaveObj = Cast<UUKSaveGame>(UGameplayStatics::CreateSaveGameObject(UUKSaveGame::StaticClass()));
	if ( !SaveObj ) return false;

	// 기존 SaveGame 필드가 있을 수 있으니, QuestProgressMap만 갱신하는 방식으로
	SaveObj->QuestProgressMap = RuntimeProgress;

	return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

bool UUKQuestManagerSubsystem::LoadFromSlot(const FString& SlotName, int32 UserIndex)
{
	if ( !UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) )
	{
		RuntimeProgress.Empty();
		return false;
	}

	UUKSaveGame* SaveObj = Cast<UUKSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if ( !SaveObj ) return false;

	RuntimeProgress = SaveObj->QuestProgressMap;
	return true;
}

bool UUKQuestManagerSubsystem::GetProgress(FName QuestId, FQuestProgress& OutProgress) const
{
	if ( const FQuestProgress* P = RuntimeProgress.Find(QuestId) )
	{
		OutProgress = *P;
		return true;
	}
	return false;
}

// 아이템 정보를 테이블에서 가져오는 함수들

FUK_ItemData* UUKQuestManagerSubsystem::GetItemData(FName ItemRowName)
{
	if (!ItemDataTable) return nullptr;

	return ItemDataTable->FindRow<FUK_ItemData>(ItemRowName, TEXT("QuestRewardLookup"));
}

void UUKQuestManagerSubsystem::GiveQuestReward(FName ItemRowName, int32 Amount)
{
	FUK_ItemData* Data = GetItemData(ItemRowName);
	if (!Data) return;

	// 인벤토리 컴포넌트 연결
	// 예: InventoryComponent->AddItem(Data, Amount);
    
	UE_LOG(LogTemp, Warning, TEXT("Reward Given: %s x%d"), *Data->ItemName.ToString(), Amount);
}