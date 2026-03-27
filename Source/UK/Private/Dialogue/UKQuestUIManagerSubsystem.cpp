#include "Dialogue/UKQuestUIManagerSubsystem.h"

#include "Quest/UKQuestManagerSubsystem.h"
#include "Quest/UKQuestObjectiveTypes.h"
#include "Dialogue/UKDialogueSubsystem.h"
#include "NPC/UK_QuestNPC.h"
#include "EngineUtils.h"
#include "Quest/DataAsset/UKQuestDefinitionAsset.h"


void UUKQuestUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if ( GetGameInstance() )
	{
		QuestSubsystem = GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>();
		DialogueSubsystem = GetGameInstance()->GetSubsystem<UUKDialogueSubsystem>();
	}
}

void UUKQuestUIManagerSubsystem::Deinitialize()
{
	QuestSubsystem = nullptr;
	DialogueSubsystem = nullptr;

	Super::Deinitialize();
}

UUKQuestManagerSubsystem* UUKQuestUIManagerSubsystem::GetQuestSubsystem() const
{
	if ( QuestSubsystem )
	{
		return QuestSubsystem;
	}

	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>() : nullptr;
}

UUKDialogueSubsystem* UUKQuestUIManagerSubsystem::GetDialogueSubsystem() const
{
	if ( DialogueSubsystem )
	{
		return DialogueSubsystem;
	}

	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UUKDialogueSubsystem>() : nullptr;
}

const UUKQuestDefinitionAsset* UUKQuestUIManagerSubsystem::GetQuestDefinitionSafe(FName QuestId) const
{
	UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
	if ( !QS || QuestId.IsNone() )
	{
		return nullptr;
	}

	return QS->GetQuestDefinition(QuestId);
}


// [4] Quest UI Text Getter

FText UUKQuestUIManagerSubsystem::GetQuestTitleText(FName QuestId) const
{
	const UUKQuestDefinitionAsset* Def = GetQuestDefinitionSafe(QuestId);
	if ( !Def )
	{
		return FText::GetEmpty();
	}

	if ( !Def->QuestTitle.IsEmpty() )
	{
		return Def->QuestTitle;
	}

	return FText::GetEmpty();
}

FText UUKQuestUIManagerSubsystem::GetQuestDescriptionText(FName QuestId) const
{
	const UUKQuestDefinitionAsset* Def = GetQuestDefinitionSafe(QuestId);
	if ( !Def )
	{
		return FText::GetEmpty();
	}

	if ( !Def->QuestDescription.IsEmpty() )
	{
		return Def->QuestDescription;
	}

	return FText::GetEmpty();
}


// [5] Quest State Bool Getter

bool UUKQuestUIManagerSubsystem::CanAcceptQuest(FName QuestId) const
{
	UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
	if ( !QS || QuestId.IsNone() )
	{
		return false;
	}

	FQuestProgress Progress;
	const bool bHasProgress = QS->GetProgress(QuestId, Progress);
	if ( bHasProgress )
	{
		return false;
	}

	return QS->CanStartQuest(QuestId);
}

bool UUKQuestUIManagerSubsystem::IsQuestInProgress(FName QuestId) const
{
	UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
	if ( !QS || QuestId.IsNone() )
	{
		return false;
	}

	FQuestProgress Progress;
	if ( !QS->GetProgress(QuestId, Progress) )
	{
		return false;
	}

	return !Progress.bCompleted;
}

bool UUKQuestUIManagerSubsystem::IsQuestCompleted(FName QuestId) const
{
	UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
	if ( !QS || QuestId.IsNone() )
	{
		return false;
	}

	FQuestProgress Progress;
	if ( !QS->GetProgress(QuestId, Progress) )
	{
		return false;
	}

	return Progress.bCompleted;
}

bool UUKQuestUIManagerSubsystem::AreObjectivesSatisfied(FName QuestId) const
{
	UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
	if ( !QS || QuestId.IsNone() )
	{
		return false;
	}

	const UUKQuestDefinitionAsset* Def = QS->GetQuestDefinition(QuestId);
	if ( !Def )
	{
		return false;
	}

	FQuestProgress Progress;
	if ( !QS->GetProgress(QuestId, Progress) )
	{
		return false;
	}

	// 목표가 하나도 없으면 false 처리
	if ( Def->Objectives.Num() == 0 )
	{
		return false;
	}

	// 현재는 QuestManager 내부 헬퍼가 protected 이므로
	// UI 매니저 쪽에서 "간단 판정"만 수행
	for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
	{
		// 1) Counter 기반 목표
		if ( !Obj.CounterName.IsNone() && Obj.RequiredCount > 0 )
		{
			const FName CounterKey(*FString::Printf(TEXT("C.%s.%s"),
				*QuestId.ToString(),
				*Obj.CounterName.ToString()));

			const int32 CurrentValue = Progress.Counters.FindRef(CounterKey);
			if ( CurrentValue < Obj.RequiredCount )
			{
				return false;
			}

			continue;
		}

		// 2) CompleteFlagCategory 기반 목표
		if ( !Obj.CompleteFlagCategory.IsNone() )
		{
			const FName FlagKey(*FString::Printf(TEXT("F.%s.%s"),
				*QuestId.ToString(),
				*Obj.CompleteFlagCategory.ToString()));

			if ( !Progress.Flags.Contains(FlagKey) )
			{
				return false;
			}

			continue;
		}

		// 3) 둘 다 없으면 아직 미지원 목표로 보고 false
		return false;
	}

	return true;
}


//[UI/Marker 용도]
//퀘스트 추적 마커 상태를 한 번에 반환
//Hidden        : 완료했거나, 아직 진행 중이 아님. 즉, 추적대상 아님
//InProgress    : 진행 중이지만 아직 완료 보고 단계는 아님
//ReadyToTurnIn : 진행 중이며 목표를 모두 달성해 완료 보고 가능


EUKQuestMarkerState UUKQuestUIManagerSubsystem::GetQuestMarkerState(FName QuestId) const
{
	if (IsQuestCompleted(QuestId))
	{
		return EUKQuestMarkerState::Hidden;
	}

	if (!IsQuestInProgress(QuestId))
	{
		return EUKQuestMarkerState::Hidden;
	}

	if (AreObjectivesSatisfied(QuestId))
	{
		return EUKQuestMarkerState::ReadyToTurnIn;
	}
	return EUKQuestMarkerState::InProgress;
}

const FUKQuestObjectiveDef* UUKQuestUIManagerSubsystem::FindFirstUnsatisfiedObjective(FName QuestId) const
{
	UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
	if ( !QS || QuestId.IsNone() )
	{
		return nullptr;
	}

	const UUKQuestDefinitionAsset* Def = QS->GetQuestDefinition(QuestId);
	if ( !Def )
	{
		return nullptr;
	}

	FQuestProgress Progress;
	if ( !QS->GetProgress(QuestId, Progress) )
	{
		return nullptr;
	}

	for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
	{
		// Counter 기반
		if ( !Obj.CounterName.IsNone() && Obj.RequiredCount > 0 )
		{
			const FName CounterKey(*FString::Printf(TEXT("C.%s.%s"),
				*QuestId.ToString(),
				*Obj.CounterName.ToString()));

			const int32 CurrentValue = Progress.Counters.FindRef(CounterKey);
			if ( CurrentValue < Obj.RequiredCount )
			{
				return &Obj;
			}

			continue;
		}

		// Flag 기반
		if ( !Obj.CompleteFlagCategory.IsNone() )
		{
			const FName FlagKey(*FString::Printf(TEXT("F.%s.%s"),
				*QuestId.ToString(),
				*Obj.CompleteFlagCategory.ToString()));

			if ( !Progress.Flags.Contains(FlagKey) )
			{
				return &Obj;
			}

			continue;
		}

		// 둘 다 없으면 미지원/미충족으로 간주
		return &Obj;
	}

	return nullptr;
}

bool UUKQuestUIManagerSubsystem::IsWarpObjectiveTarget(const FName& TargetId) const
{
	if ( TargetId.IsNone() )
	{
		return false;
	}

	const FString TargetStr = TargetId.ToString();

	return TargetStr.Contains(TEXT("Warp"), ESearchCase::IgnoreCase)
		|| TargetStr.Contains(TEXT("WRP"), ESearchCase::IgnoreCase)
		|| TargetStr.Equals(TEXT("WarpUnlocked"), ESearchCase::IgnoreCase);
}

EUKQuestMarkerTargetType UUKQuestUIManagerSubsystem::GetQuestMarkerTargetType(FName QuestId) const
{
	const EUKQuestMarkerState MarkerState = GetQuestMarkerState(QuestId);

	if ( MarkerState == EUKQuestMarkerState::Hidden )
	{
		return EUKQuestMarkerTargetType::None;
	}

	// 완료 보고 가능 상태면 무조건 보고 NPC
	if ( MarkerState == EUKQuestMarkerState::ReadyToTurnIn )
	{
		return EUKQuestMarkerTargetType::NPC;
	}

	// 진행 중이면 "현재 미충족 Objective"를 보고 어디로 띄울지 판단
	const FUKQuestObjectiveDef* PendingObj = FindFirstUnsatisfiedObjective(QuestId);
	if ( !PendingObj )
	{
		// 안전 fallback
		return EUKQuestMarkerTargetType::NPC;
	}

	switch ( PendingObj->Type )
	{
	case EUKQuestObjectiveType::TalkedTo:
		return EUKQuestMarkerTargetType::NPC;

	case EUKQuestObjectiveType::Killed:
		return EUKQuestMarkerTargetType::MonsterSpawner;

	case EUKQuestObjectiveType::Custom:
	{
		UUKQuestManagerSubsystem* QS = GetQuestSubsystem();
		if ( QS )
		{
			const UUKQuestDefinitionAsset* Def = QS->GetQuestDefinition(QuestId);
			if ( Def && Def->Tag == EUKQuestTag::WRP )
			{
				return EUKQuestMarkerTargetType::Warp;
			}
		}

		if ( IsWarpObjectiveTarget(PendingObj->TargetId) )
		{
			return EUKQuestMarkerTargetType::Warp;
		}
		return EUKQuestMarkerTargetType::NPC;
	}

	case EUKQuestObjectiveType::EnteredZone:
	case EUKQuestObjectiveType::GotItem:
	case EUKQuestObjectiveType::Delivered:
	default:
		// 현재 데이터 구조상 별도 단말 정보가 없으므로 안전 fallback
		return EUKQuestMarkerTargetType::NPC;
	}
}

FName UUKQuestUIManagerSubsystem::ResolveReportNpcId(FName QuestId) const
{
	if ( QuestId.IsNone() )
	{
		return NAME_None;
	}

	UWorld* World = GetWorld();
	if ( !World )
	{
		return NAME_None;
	}

	for ( TActorIterator<AUK_QuestNPC> It(World); It; ++It )
	{
		AUK_QuestNPC* NPC = *It;
		if ( !IsValid(NPC) )
		{
			continue;
		}

		if ( NPC->GetReportQuestIDs().Contains(QuestId) )
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[MarkerRoute][ResolveReportNpcId] QuestId=%s -> NPCID=%s NPC=%s"),
				*QuestId.ToString(),
				*NPC->GetNPCID().ToString(),
				*NPC->GetName());
			return NPC->GetNPCID();
		}
	}

	return NAME_None;
}

FName UUKQuestUIManagerSubsystem::ResolveOfferNpcId(FName QuestId) const
{
	if ( QuestId.IsNone() )
	{
		return NAME_None;
	}

	UWorld* World = GetWorld();
	if ( !World )
	{
		return NAME_None;
	}

	for ( TActorIterator<AUK_QuestNPC> It(World); It; ++It )
	{
		AUK_QuestNPC* NPC = *It;
		if ( !IsValid(NPC) )
		{
			continue;
		}

		if ( !NPC->GetQuestID().IsNone() && NPC->GetQuestID() == QuestId )
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[MarkerRoute][ResolveOfferNpcId] QuestId=%s -> NPCID=%s NPC=%s"),
				*QuestId.ToString(),
				*NPC->GetNPCID().ToString(),
				*NPC->GetName());
			return NPC->GetNPCID();
		}

		if ( NPC->GetOfferQuestIDs().Contains(QuestId) )
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[MarkerRoute][ResolveOfferNpcId] QuestId=%s -> NPCID=%s NPC=%s"),
				*QuestId.ToString(),
				*NPC->GetNPCID().ToString(),
				*NPC->GetName());
			return NPC->GetNPCID();
		}
	}
UE_LOG(LogTemp, Warning,
	TEXT("[MarkerRoute][ResolveOfferNpcId] QuestId=%s -> NOT FOUND"),
	*QuestId.ToString());
	return NAME_None;
}

FName UUKQuestUIManagerSubsystem::GetQuestMarkerTargetId(FName QuestId, EUKQuestMarkerTargetType TargetType) const
{
	if ( QuestId.IsNone() || TargetType == EUKQuestMarkerTargetType::None )
	{
		return NAME_None;
	}

	const EUKQuestMarkerState MarkerState = GetQuestMarkerState(QuestId);

	// 완료 보고 가능이면 "보고 NPC"
	if ( MarkerState == EUKQuestMarkerState::ReadyToTurnIn )
	{
		if ( TargetType == EUKQuestMarkerTargetType::NPC )
		{
			return ResolveReportNpcId(QuestId);
		}

		return NAME_None;
	}

	// 진행 중이면 미충족 Objective 기준
	const FUKQuestObjectiveDef* PendingObj = FindFirstUnsatisfiedObjective(QuestId);
	if ( !PendingObj )
	{
		return NAME_None;
	}

	switch ( TargetType )
	{
	case EUKQuestMarkerTargetType::NPC:
		// TalkedTo는 Objective TargetId가 실제 대상 NPC일 수 있으므로 우선 사용
		if ( PendingObj->Type == EUKQuestObjectiveType::TalkedTo && !PendingObj->TargetId.IsNone() )
		{
			return PendingObj->TargetId;
		}

		// Delivered / Custom(제작 등) / 기타 진행형은 현재 구조상 진행 NPC로 처리
		return ResolveOfferNpcId(QuestId);

	case EUKQuestMarkerTargetType::Warp:
		// 현재 구조상 Warp 목적지는 Objective TargetId 자체를 Warp 식별자로 사용
		return PendingObj->TargetId;

	case EUKQuestMarkerTargetType::MonsterSpawner:
		// 현재는 스포너 ID 매핑 구조가 아직 없으므로 일단 Objective TargetId를 넘김
		// 다음 단계에서 MonsterTargetId -> SpawnerID 매핑이 필요
		return PendingObj->TargetId;

	case EUKQuestMarkerTargetType::None:
	default:
		return NAME_None;
	}
}

FUKQuestMarkerRouteInfo UUKQuestUIManagerSubsystem::GetQuestMarkerRouteInfo(FName QuestId) const
{
	FUKQuestMarkerRouteInfo OutInfo;
	OutInfo.QuestId = QuestId;
	OutInfo.MarkerState = GetQuestMarkerState(QuestId);
	OutInfo.TargetType = GetQuestMarkerTargetType(QuestId);
	OutInfo.TargetId = GetQuestMarkerTargetId(QuestId, OutInfo.TargetType);

	UE_LOG(LogTemp, Warning,
		TEXT("[MarkerRoute][RouteInfo] QuestId=%s State=%d Type=%d TargetId=%s"),
		*OutInfo.QuestId.ToString(),
		static_cast< int32 >( OutInfo.MarkerState ),
		static_cast< int32 >( OutInfo.TargetType ),
		*OutInfo.TargetId.ToString());
	return OutInfo;
}

// [6] Dialogue UI Getter

FText UUKQuestUIManagerSubsystem::GetCurrentDialogueSpeakerName() const
{
	UUKDialogueSubsystem* DS = GetDialogueSubsystem();
	if ( !DS )
	{
		return FText::GetEmpty();
	}

	return DS->GetCurrentSpeakerName();
}

FText UUKQuestUIManagerSubsystem::GetCurrentDialogueText() const
{
	UUKDialogueSubsystem* DS = GetDialogueSubsystem();
	if ( !DS )
	{
		return FText::GetEmpty();
	}

	return DS->GetCurrentDialogueText();
}

TArray<FText> UUKQuestUIManagerSubsystem::GetCurrentDialogueChoiceTexts() const
{
	UUKDialogueSubsystem* DS = GetDialogueSubsystem();
	if ( !DS )
	{
		return TArray<FText>{};
	}

	return DS->GetCurrentChoiceTexts();
}

FUKCurrentDialogueUIData UUKQuestUIManagerSubsystem::GetCurrentDialogueUIData() const
{
	UUKDialogueSubsystem* DS = GetDialogueSubsystem();
	if ( !DS )
	{
		return FUKCurrentDialogueUIData{};
	}

	return DS->GetCurrentDialogueUIData();
}

bool UUKQuestUIManagerSubsystem::SelectDialogueChoice(int32 ChoiceIndex)
{
	UUKDialogueSubsystem* DS = GetDialogueSubsystem();
	if ( !DS )
	{
		return false;
	}

	return DS->SelectChoice(ChoiceIndex);
}