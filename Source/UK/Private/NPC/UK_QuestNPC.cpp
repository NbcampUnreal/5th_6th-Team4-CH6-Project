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
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "UI/InGame/UK_CheckPoint.h"

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
	InteractionSphere->SetSphereRadius(500.f); // 마커 띄울 범위 임시 설정

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerExit);

	MarkerComp = CreateDefaultSubobject<UChildActorComponent>("MarkerComp");
	MarkerComp->SetupAttachment(RootComponent);
	MarkerComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

}

void AUK_QuestNPC::BeginPlay()
{
	Super::BeginPlay();

	bPlayerInRange = false;

	// 체크포인트(거리측정) 관련
	if ( SelectMarker )
	{
		MarkerComp->SetChildActorClass(SelectMarker);
		MarkerComp->CreateChildActor();

		NPCMarker = Cast<AUK_CheckPoint>(MarkerComp->GetChildActor());
		if ( NPCMarker )
		{
			FVector CheckLocation = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
			NPCMarker->SetActorLocation(CheckLocation);
		}

	}

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

	UE_LOG(LogTemp, Log, TEXT("[NPC] Loaded from DT. NPCID=%s"),*NPCID.ToString());

	// 추가: 퀘스트 상태 변화 바인딩
	QuestSys->OnQuestMarkerResetRequested.AddDynamic(this, &AUK_QuestNPC::HandleQuestMarkerResetRequested);
	QuestSys->OnQuestMarkerRouteResolved.AddDynamic(this, &AUK_QuestNPC::HandleQuestMarkerRouteResolved);

	UE_LOG(LogTemp, Warning, TEXT("[QuestMarker] Bound Marker terminal broadcasts | NPC=%s | NPCID=%s"),*GetName(),*NPCID.ToString());
	// 추가: 시작 시점 마커 상태 초기화
	RefreshQuestMarker();
}

void AUK_QuestNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if ( UWorld* World = GetWorld() )
	{
		if ( UGameInstance* GI = World->GetGameInstance() )
		{
			if ( UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
			{
				QuestSys->OnQuestMarkerResetRequested.RemoveDynamic(this, &AUK_QuestNPC::HandleQuestMarkerResetRequested);
				QuestSys->OnQuestMarkerRouteResolved.RemoveDynamic(this, &AUK_QuestNPC::HandleQuestMarkerRouteResolved);
			}
		}
	}

	GetWorldTimerManager().ClearTimer(MarkerTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AUK_QuestNPC::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);

	if ( Player && Player->InteractionComp )
	{
		bPlayerInRange = true;

		Player->InteractionComp->SetNearActor(this);

		RefreshQuestMarker();
	}
}

void AUK_QuestNPC::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);

	if ( Player && Player->InteractionComp )
	{
		bPlayerInRange = false;

		Player->InteractionComp->ClearNearActor();

		RefreshQuestMarker();
	}
}

void AUK_QuestNPC::UpdateMarkerRotation()
{
	if ( !NPCMarker )
	{
		return;
	}

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if ( !PlayerChar )
	{
		return;
	}

	FVector ToPlayer = PlayerChar->GetActorLocation() - NPCMarker->GetActorLocation();
	FRotator LookAtRotation = ToPlayer.Rotation();
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Yaw += 90.f;

	NPCMarker->SetActorRotation(LookAtRotation);
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
		const bool bReportStarted = QuestSys->GetProgress(CandidateId, Progress);

		if ( bReportStarted && !Progress.bCompleted )
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
		UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] TryProcessDelivery failed: QuestDefinition is null. Quest=%s"),
			*QuestId.ToString());
		return false;
	}

	FQuestProgress Progress;
	if ( !QuestSys->GetProgress(QuestId, Progress) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] TryProcessDelivery failed: Progress not found. Quest=%s"),
			*QuestId.ToString());
		return false;
	}

	// 이미 완료된 퀘스트면 전달 처리 안 함
	if ( Progress.bCompleted )
	{
		UE_LOG(LogTemp, Log, TEXT("[QuestNPC] TryProcessDelivery skipped: already completed. Quest=%s"),
			*QuestId.ToString());
		return false;
	}

	UUK_InventoryComponent* InventoryComp = Player->FindComponentByClass<UUK_InventoryComponent>();
	if ( !InventoryComp )
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] InventoryComponent not found."));
		return false;
	}

	bool bAnyDelivered = false;
	bool bFoundDeliveredObjective = false;

	for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
	{
		// Delivered 타입만 처리
		if ( Obj.Type != EUKQuestObjectiveType::Delivered )
		{
			continue;
		}

		bFoundDeliveredObjective = true;

		if ( Obj.TargetId.IsNone() || Obj.RequiredCount <= 0 )
		{
			UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] Invalid Delivered objective skipped. Quest=%s Objective=%s"),
				*QuestId.ToString(),
				*Obj.ObjectiveId.ToString());
			continue;
		}

		// 이미 이 목표가 충족된 상태면 다시 전달하지 않음
		if ( !Obj.CounterName.IsNone() )
		{
			const FName CounterKey = MakeQuestCounterKey_Local(QuestId, Obj.CounterName);
			const int32 CurrentDeliveredCount = Progress.Counters.FindRef(CounterKey);

			if ( CurrentDeliveredCount >= Obj.RequiredCount )
			{
				UE_LOG(LogTemp, Log, TEXT("[QuestNPC] Delivered objective already satisfied. Quest=%s Objective=%s Counter=%s Current=%d Required=%d"),
					*QuestId.ToString(),
					*Obj.ObjectiveId.ToString(),
					*Obj.CounterName.ToString(),
					CurrentDeliveredCount,
					Obj.RequiredCount);
				continue;
			}
		}
		else if ( !Obj.CompleteFlagCategory.IsNone() )
		{
			const FName FlagKey = MakeQuestFlagKey_Local(QuestId, Obj.CompleteFlagCategory);
			if ( Progress.Flags.Contains(FlagKey) )
			{
				UE_LOG(LogTemp, Log, TEXT("[QuestNPC] Delivered flag objective already satisfied. Quest=%s Objective=%s Flag=%s"),
					*QuestId.ToString(),
					*Obj.ObjectiveId.ToString(),
					*Obj.CompleteFlagCategory.ToString());
				continue;
			}
		}

		// 현재 보유 수량 확인
		const int32 CurrentCount = InventoryComp->GetItemTotalQuantity(Obj.TargetId);
		if ( CurrentCount < Obj.RequiredCount )
		{
			UE_LOG(LogTemp, Log, TEXT("[QuestNPC] Delivery failed. Quest=%s Objective=%s Need=%d Have=%d Item=%s"),
				*QuestId.ToString(),
				*Obj.ObjectiveId.ToString(),
				Obj.RequiredCount,
				CurrentCount,
				*Obj.TargetId.ToString());
			continue;
		}

		// 아이템 차감
		const int32 RemoveResult = InventoryComp->RemoveItem(Obj.TargetId, Obj.RequiredCount);
		if ( RemoveResult != 0 )
		{
			UE_LOG(LogTemp, Warning, TEXT("[QuestNPC] RemoveItem failed. Quest=%s Objective=%s Item=%s Need=%d RemainNotRemoved=%d"),
				*QuestId.ToString(),
				*Obj.ObjectiveId.ToString(),
				*Obj.TargetId.ToString(),
				Obj.RequiredCount,
				RemoveResult);
			continue;
		}

		// 전달 성공 -> Delivered 이벤트를 개수만큼 발사
		for ( int32 i = 0; i < Obj.RequiredCount; ++i )
		{
			const FString EventStr = FString::Printf(TEXT("QuestEvent.Delivered.%s"), *Obj.TargetId.ToString());
			QuestSys->EmitQuestEvent(FName(*EventStr));
		}

		UE_LOG(LogTemp, Log, TEXT("[QuestNPC] Delivery success. Quest=%s Objective=%s Item=%s Count=%d"),
			*QuestId.ToString(),
			*Obj.ObjectiveId.ToString(),
			*Obj.TargetId.ToString(),
			Obj.RequiredCount);

		bAnyDelivered = true;

		// 방금 EmitQuestEvent로 Progress가 바뀌었을 수 있으니 다시 읽기
		QuestSys->GetProgress(QuestId, Progress);
	}

	if ( !bFoundDeliveredObjective )
	{
		UE_LOG(LogTemp, Log, TEXT("[QuestNPC] TryProcessDelivery: no Delivered objectives. Quest=%s"),
			*QuestId.ToString());
	}

	return bAnyDelivered;
}

void AUK_QuestNPC::Interact(AActor* Interactor)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0F, FColor::Red, TEXT("F-상호작용(NPC::Interact) 입력됨"));
	}
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Interactor);
	if (!Player) return;

	HandleQuestInteract(Player);
}

void AUK_QuestNPC::HandleQuestInteract(AUK_CharacterBase* Player)
{
	if (!Player)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if ( !GI )
	{
		return;
	}

	UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if (!QuestSys)
	{
		return;
	}

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());
	if (!PlayerCtl)
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

void AUK_QuestNPC::RefreshQuestMarker()
{
	if ( !MarkerComp || !NPCMarker )
	{
		return;
	}

	// 가까우면 무조건 숨김
	if ( bPlayerInRange )
	{
		HideMarkerInternal();
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

	UUKQuestUIManagerSubsystem* QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>();
	if ( !QuestUIManager )
	{
		return;
	}

	// 현재 이 NPC가 담당 중인 후보 퀘스트들을 기준으로
	// 머리(RouteInfo)가 "NPC + 내 NPCID"를 가리키는 퀘스트가 있는지 확인
	TArray<FName> CandidateQuestIds;

	if ( !QuestID.IsNone() )
	{
		CandidateQuestIds.Add(QuestID);
	}

	CandidateQuestIds.Append(OfferQuestIDs);
	CandidateQuestIds.Append(ReportQuestIDs);

	for ( const FName& CandidateQuestId : CandidateQuestIds )
	{
		if ( CandidateQuestId.IsNone() )
		{
			continue;
		}

		const FUKQuestMarkerRouteInfo RouteInfo = QuestUIManager->GetQuestMarkerRouteInfo(CandidateQuestId);

		if ( RouteInfo.TargetType == EUKQuestMarkerTargetType::NPC &&
			RouteInfo.TargetId == NPCID &&
			RouteInfo.MarkerState != EUKQuestMarkerState::Hidden )
		{
			ShowMarkerInternal(RouteInfo.MarkerState);
			return;
		}
	}

	// 해당되는 것이 없으면 숨김
	HideMarkerInternal();
}

void AUK_QuestNPC::HideMarkerInternal()
{
	if ( !MarkerComp || !NPCMarker )
	{
		return;
	}

	MarkerComp->SetVisibility(false, true);
	NPCMarker->SetActorHiddenInGame(true);
	GetWorldTimerManager().ClearTimer(MarkerTimerHandle);
}

void AUK_QuestNPC::ShowMarkerInternal(EUKQuestMarkerState MarkerState)
{
	if ( !MarkerComp || !NPCMarker )
	{
		return;
	}

	if ( MarkerState == EUKQuestMarkerState::Hidden )
	{
		HideMarkerInternal();
		return;
	}

	MarkerComp->SetVisibility(true, true);
	NPCMarker->SetActorHiddenInGame(false);

	GetWorldTimerManager().SetTimer(
		MarkerTimerHandle,
		this,
		&AUK_QuestNPC::UpdateMarkerRotation,
		0.03f,
		true
	);
}

void AUK_QuestNPC::HandleQuestMarkerResetRequested()
{
	HideMarkerInternal();
}

void AUK_QuestNPC::HandleQuestMarkerRouteResolved(FName QuestId, EUKQuestMarkerTargetType TargetType, FName TargetId)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[QuestMarker][NPC Terminal] RouteResolved | NPC=%s NPCID=%s QuestId=%s TargetType=%d TargetId=%s"),
		*GetName(),
		*NPCID.ToString(),
		*QuestId.ToString(),
		static_cast< int32 >( TargetType ),
		*TargetId.ToString());

	// NPC 단말이 아니면 무시
	if ( TargetType != EUKQuestMarkerTargetType::NPC )
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[NPC Terminal] Ignored: not NPC target. NPCID=%s QuestId=%s Type=%d TargetId=%s"),
			*NPCID.ToString(),
			*QuestId.ToString(),
			static_cast< int32 >( TargetType ),
			*TargetId.ToString());
		return;
	}

	// 내 NPCID와 일치하지 않으면 무시
	if ( TargetId.IsNone() || TargetId != NPCID )
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[NPC Terminal] Ignored: TargetId mismatch. MyNPCID=%s QuestId=%s TargetId=%s"),
			*NPCID.ToString(),
			*QuestId.ToString(),
			*TargetId.ToString());
		return;
	}

	// 플레이어가 가까우면 숨김 유지
	if ( bPlayerInRange )
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[NPC Terminal] Hidden because player in range. NPCID=%s QuestId=%s"),
			*NPCID.ToString(),
			*QuestId.ToString());
		HideMarkerInternal();
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

	UUKQuestUIManagerSubsystem* QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>();
	if ( !QuestUIManager )
	{
		return;
	}

	const EUKQuestMarkerState MarkerState = QuestUIManager->GetQuestMarkerState(QuestId);
	ShowMarkerInternal(MarkerState);
}