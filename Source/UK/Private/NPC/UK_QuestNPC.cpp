#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Dialogue/UKDialogueSubsystem.h"
#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/Component/UK_QuestComponent.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "ActorComponent/UK_InventoryComponent.h"

#include "Blueprint/UserWidget.h"       
#include "GameFramework/PlayerController.h"

namespace
{
	static FName MakeQuestCounterKey_Local(FName QuestId, FName CounterName)
	{
		return FName(*FString::Printf(TEXT("C.%s.%s"),
			*QuestId.ToString(),
			*CounterName.ToString()));
	}

	static FName MakeQuestFlagKey_Local(FName QuestId, FName Category)
	{
		return FName(*FString::Printf(TEXT("F.%s.%s"),
			*QuestId.ToString(),
			*Category.ToString()));
	}
}

AUK_QuestNPC::AUK_QuestNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	bPlayerInRange = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(1000.f); // 마커 띄울 범위 임시 설정

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerExit);

	QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
	QuestMarker->SetupAttachment(RootComponent);
	QuestMarker->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	QuestMarker->SetVisibility(false);

}

void AUK_QuestNPC::BeginPlay()
{
	Super::BeginPlay();

	bPlayerInRange = false;

	// NPCID는 BP 기본값 또는 배치된 액터에서 미리 지정되어 있어야 함
	if ( NPCID.IsNone() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] NPCID is None."));
		return;
	}

	UWorld* World = GetWorld();
	if ( !World )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] World is null."));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if ( !GI )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] GameInstance is null."));
		return;
	}

	UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSys )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] QuestManagerSubsystem is null."));
		return;
	}

	// NPCID로 서브시스템에서 NPC 데이터 조회
	const FUK_NPCData* Row = QuestSys->GetNPCDataByNPCID(NPCID);
	if ( !Row )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] NPC data not found for NPCID=%s"), *NPCID.ToString());
		return;
	}

	// DT에서 읽어온 값 복사
	NPCDisplayName = Row->NPCName;
	NPCDescription = Row->NPCDescription;

	UE_LOG(LogTemp, Log, TEXT("[NPC] Loaded from DT. NPCID=%s"),
		*NPCID.ToString());
}


void AUK_QuestNPC::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);

	if ( Player && Player->InteractionComp )
	{
		bPlayerInRange = true;

		Player->InteractionComp->SetNearActor(this);

		if ( QuestMarker )
		{
			QuestMarker->SetVisibility(true);
			GetWorldTimerManager().SetTimer(
				MarkerTimerHandle,
				this,
				&AUK_QuestNPC::UpdateMarkerRotation,
				0.03f,
				true
			);
		}
	}
}

void AUK_QuestNPC::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);

	if ( Player && Player->InteractionComp )

	{
		bPlayerInRange = false;

		Player->InteractionComp->ClearNearActor();

		if ( QuestMarker )
		{
			QuestMarker->SetVisibility(false);
			GetWorldTimerManager().ClearTimer(MarkerTimerHandle);
		}
	}
}

void AUK_QuestNPC::UpdateMarkerRotation()
{
	if ( !QuestMarker ) return;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if ( !PlayerChar ) return;

	FVector ToPlayer = PlayerChar->GetActorLocation() - QuestMarker->GetComponentLocation();
	FRotator LookAtRotation = ToPlayer.Rotation();
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Yaw += 90.f;
	QuestMarker->SetWorldRotation(LookAtRotation);
}

bool AUK_QuestNPC::IsQuestStarted(UUKQuestManagerSubsystem* QuestSys, FName InQuestId) const
{
	if ( !QuestSys || InQuestId.IsNone() )
	{
		return false;
	}

	FQuestProgress Progress;
	return QuestSys->GetProgress(InQuestId, Progress);
}

bool AUK_QuestNPC::IsQuestCompleted(UUKQuestManagerSubsystem* QuestSys, FName InQuestId) const
{
	if ( !QuestSys || InQuestId.IsNone() )
	{
		return false;
	}

	FQuestProgress Progress;
	if ( !QuestSys->GetProgress(InQuestId, Progress) )
	{
		return false;
	}

	return Progress.bCompleted;
}

FName AUK_QuestNPC::ResolveQuestIdToShow(UUKQuestManagerSubsystem* QuestSys) const
{
	if ( !QuestSys )
	{
		return NAME_None;
	}

	// 0) fallback 단일 퀘스트
	if ( !QuestID.IsNone() )
	{
		FQuestProgress FallbackProgress;
		const bool bStarted = QuestSys->GetProgress(QuestID, FallbackProgress);
		const bool bCanStart = QuestSys->CanStartQuest(QuestID);

		if ( ( bStarted && !FallbackProgress.bCompleted ) || ( !bStarted && bCanStart ) )
		{
			return QuestID;
		}
	}

	// 1) 보고/완료 담당 퀘스트 우선
	for ( const FName& CandidateId : ReportQuestIDs )
	{
		FQuestProgress Progress;
		if ( QuestSys->GetProgress(CandidateId, Progress) && !Progress.bCompleted )
		{
			UE_LOG(LogTemp, Log, TEXT("[QuestNPC] ReportQuest picked. NPCID=%s Quest=%s"),
				*NPCID.ToString(), *CandidateId.ToString());
			return CandidateId;
		}
	}

	// 2) 새로 발급 가능한 퀘스트
	for ( const FName& CandidateId : OfferQuestIDs )
	{
		FQuestProgress Progress;
		const bool bStarted = QuestSys->GetProgress(CandidateId, Progress);
		const bool bCanStart = QuestSys->CanStartQuest(CandidateId);

		if ( !bStarted && bCanStart )
		{
			UE_LOG(LogTemp, Log, TEXT("[QuestNPC] OfferQuest picked. NPCID=%s Quest=%s"),
				*NPCID.ToString(), *CandidateId.ToString());
			return CandidateId;
		}
	}

	return NAME_None;
}

FName AUK_QuestNPC::MakeDialogueIdBySuffix(const UUKQuestDefinitionAsset* Def, const FString& Suffix) const
{
	if ( !Def || Def->DialogueId.IsNone() )
	{
		return NAME_None;
	}

	FString DialogueIdStr = Def->DialogueId.ToString();

	if ( DialogueIdStr.EndsWith(TEXT("_Offer")) )
	{
		DialogueIdStr.LeftChopInline(6);
	}
	else if ( DialogueIdStr.EndsWith(TEXT("_Complete")) )
	{
		DialogueIdStr.LeftChopInline(9);
	}
	else if ( DialogueIdStr.EndsWith(TEXT("_InProgress")) )
	{
		DialogueIdStr.LeftChopInline(11);
	}

	DialogueIdStr += Suffix;
	return FName(*DialogueIdStr);
}

FText AUK_QuestNPC::BuildInProgressDialogueText(UUKQuestManagerSubsystem* QuestSys, const UUKQuestDefinitionAsset* Def, FName InQuestId) const
{
	if ( !QuestSys || !Def || InQuestId.IsNone() )
	{
		return FText::FromString(TEXT("아직 목표를 모두 달성하지 못했습니다."));
	}

	return FText::FromString(TEXT("아직 목표를 모두 달성하지 못했습니다. 진행 상황을 확인해 주세요."));
}

FText AUK_QuestNPC::BuildInProgressQuestDesc(UUKQuestManagerSubsystem* QuestSys, const UUKQuestDefinitionAsset* Def, FName InQuestId) const
{
	if ( !QuestSys || !Def || InQuestId.IsNone() )
	{
		return FText::GetEmpty();
	}

	FQuestProgress Progress;
	if ( !QuestSys->GetProgress(InQuestId, Progress) )
	{
		return Def->QuestDescription;
	}

	FString Result;

	if ( !Def->QuestDescription.IsEmpty() )
	{
		Result += Def->QuestDescription.ToString();
		Result += TEXT("\n\n");
	}

	Result += TEXT("[진행 상황]");

	for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
	{
		FString Label;

		if ( !Obj.TargetId.IsNone() )
		{
			Label = Obj.TargetId.ToString();
		}
		else if ( !Obj.ObjectiveId.IsNone() )
		{
			Label = Obj.ObjectiveId.ToString();
		}
		else
		{
			Label = TEXT("Objective");
		}

		// Counter 기반 목표
		if ( !Obj.CounterName.IsNone() && Obj.RequiredCount > 0 )
		{
			const FName CounterKey = MakeQuestCounterKey_Local(InQuestId, Obj.CounterName);
			const int32 CurrentValue = Progress.Counters.FindRef(CounterKey);

			Result += FString::Printf(
				TEXT("\n- %s : %d / %d"),
				*Label,
				CurrentValue,
				Obj.RequiredCount
			);
			continue;
		}

		// Flag 기반 목표
		if ( !Obj.CompleteFlagCategory.IsNone() )
		{
			const FName FlagKey = MakeQuestFlagKey_Local(InQuestId, Obj.CompleteFlagCategory);
			const bool bDone = Progress.Flags.Contains(FlagKey);

			Result += FString::Printf(
				TEXT("\n- %s : %s"),
				*Label,
				bDone ? TEXT("완료") : TEXT("미완료")
			);
			continue;
		}

		// 둘 다 없으면 기본 문구
		Result += FString::Printf(
			TEXT("\n- %s : 진행 중"),
			*Label
		);
	}

	return FText::FromString(Result);
}

FName AUK_QuestNPC::ResolveDialogueIdForQuest(UUKQuestManagerSubsystem* QuestSys, const UUKQuestDefinitionAsset* Def, FName InQuestId) const
{
	if ( !QuestSys || !Def || InQuestId.IsNone() )
	{
		return NAME_None;
	}

	FQuestProgress Progress;
	const bool bStarted = QuestSys->GetProgress(InQuestId, Progress);

	// 아직 시작 안 한 퀘스트 -> Offer 대화
	if ( !bStarted )
	{
		return Def->DialogueId;
	}

	// 이미 완료됨 -> 여기서는 다루지 않음
	if ( Progress.bCompleted )
	{
		return NAME_None;
	}

	// 목표를 모두 만족했으면 Complete 대화
	if ( QuestSys->AreObjectivesSatisfied(InQuestId) )
	{
		return MakeDialogueIdBySuffix(Def, TEXT("_Complete"));
	}

	// 진행 중이지만 목표 미달성 -> InProgress 대화 시도
	return MakeDialogueIdBySuffix(Def, TEXT("_InProgress"));
}

// DLV 퀘스트에 필요한 세팅
bool AUK_QuestNPC::TryProcessDelivery(UUKQuestManagerSubsystem* QuestSys, AUK_CharacterBase* Player, FName QuestId) const
{
	if ( !QuestSys || !Player || QuestId.IsNone() )
	{
		return false;
	}

	const UUKQuestDefinitionAsset* Def = QuestSys->GetQuestDefinition(QuestId);
	if ( !Def )
	{
		return false;
	}

	FQuestProgress Progress;
	if ( !QuestSys->GetProgress(QuestId, Progress) )
	{
		return false;
	}

	// 이미 완료된 퀘스트면 전달 처리 안 함
	if ( Progress.bCompleted )
	{
		return false;
	}

	// 이미 목표 만족 상태면 다시 전달 처리 안 함
	if ( QuestSys->AreObjectivesSatisfied(QuestId) )
	{
		return false;
	}

	// 현재는 Objective 1개 기준 처리
	if ( Def->Objectives.Num() <= 0 )
	{
		return false;
	}

	const FUKQuestObjectiveDef& Obj = Def->Objectives[ 0 ];

	// 전달 퀘스트만 처리
	if ( Obj.Type != EUKQuestObjectiveType::Delivered )
	{
		return false;
	}

	if ( Obj.TargetId.IsNone() || Obj.RequiredCount <= 0 )
	{
		return false;
	}

	// 플레이어 인벤토리 컴포넌트 찾기
	UUK_InventoryComponent* InventoryComp = Player->FindComponentByClass<UUK_InventoryComponent>();
	if ( !InventoryComp )
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] InventoryComponent not found."));
		return false;
	}

	// 현재 보유 수량 확인
	const int32 CurrentCount = InventoryComp->GetItemTotalQuantity(Obj.TargetId);
	if ( CurrentCount < Obj.RequiredCount )
	{
		UE_LOG(LogTemp, Log, TEXT("[QuestNPC] Delivery failed. Need=%d Have=%d Item=%s"),
			Obj.RequiredCount,
			CurrentCount,
			*Obj.TargetId.ToString());
		return false;
	}

	// 아이템 차감
	const int32 RemoveResult = InventoryComp->RemoveItem(Obj.TargetId, Obj.RequiredCount);
	if ( RemoveResult != 0 )
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] RemoveItem failed. Item=%s Need=%d RemainNotRemoved=%d"),
			*Obj.TargetId.ToString(),
			Obj.RequiredCount,
			RemoveResult);
		return false;
	}

	// 전달 성공 -> Delivered 이벤트를 개수만큼 발사
	for ( int32 i = 0; i < Obj.RequiredCount; ++i )
	{
		const FString EventStr = FString::Printf(TEXT("QuestEvent.Delivered.%s"), *Obj.TargetId.ToString());
		QuestSys->EmitQuestEvent(FName(*EventStr));
	}

	UE_LOG(LogTemp, Log, TEXT("[QuestNPC] Delivery success. Quest=%s Item=%s Count=%d"),
		*QuestId.ToString(),
		*Obj.TargetId.ToString(),
		Obj.RequiredCount);

	return true;
}

void AUK_QuestNPC::Interact(AActor* Interactor)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Interactor);
	if ( !Player ) return;

	HandleQuestInteract(Player);
}

void AUK_QuestNPC::HandleQuestInteract(AUK_CharacterBase* Player)
{
	if ( !Player || !bPlayerInRange )
	{
		return;
	}

	UWorld* World = GetWorld();
	if ( !World )
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if ( !GI )
	{
		return;
	}

	UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSys )
	{
		return;
	}

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());
	if ( !PlayerCtl )
	{
		return;
	}

	const FName ActiveQuestId = ResolveQuestIdToShow(QuestSys);
	if ( ActiveQuestId.IsNone() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] ActiveQuestId is None. NPCID=%s"), *NPCID.ToString());
		return;
	}

	// DLV 퀘스트면 여기서 먼저 전달 처리 시도
	TryProcessDelivery(QuestSys, Player, ActiveQuestId);

	const UUKQuestDefinitionAsset* Def = QuestSys->GetQuestDefinition(ActiveQuestId);
	if ( !Def )
	{
		return;
	}

	FText QuestTitle = Def->QuestTitle;
	FText Dialogue = FText::GetEmpty();
	FText QuestDesc = Def->QuestDescription;

	FQuestProgress Progress;
	const bool bStarted = QuestSys->GetProgress(ActiveQuestId, Progress);
	const bool bCompleted = bStarted && Progress.bCompleted;
	const bool bObjectivesSatisfied = QuestSys->AreObjectivesSatisfied(ActiveQuestId);

	// 진행 중 + 미달성 상태에서는 정적 fallback 문구를 강제로 준비
	if ( bStarted && !bCompleted && !bObjectivesSatisfied )
	{
		Dialogue = BuildInProgressDialogueText(QuestSys, Def, ActiveQuestId);
		QuestDesc = BuildInProgressQuestDesc(QuestSys, Def, ActiveQuestId);
	}

	const FName DialogueIdToStart = ResolveDialogueIdForQuest(QuestSys, Def, ActiveQuestId);

	if ( !DialogueIdToStart.IsNone() )
	{
		UUKDialogueSubsystem* DialogueSys = GI->GetSubsystem<UUKDialogueSubsystem>();
		if ( DialogueSys )
		{
			FString PackFileName = Def->DialoguePackId.ToString();

			if ( !PackFileName.EndsWith(TEXT(".json")) )
			{
				PackFileName += TEXT(".json");
			}

			const bool bStartedDialogue = DialogueSys->StartDialogue(PackFileName, DialogueIdToStart);
			if ( !bStartedDialogue )
			{
				UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] StartDialogue failed. Pack=%s DialogueId=%s"),
					*PackFileName,
					*DialogueIdToStart.ToString());
			}
		}
	}

	PlayerCtl->ShowQuestUI(
		ActiveQuestId,
		QuestTitle,
		Dialogue,
		QuestDesc
	);
}