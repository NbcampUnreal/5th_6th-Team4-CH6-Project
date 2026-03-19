#include "Dialogue/UKQuestUIManagerSubsystem.h"

#include "Quest/UKQuestManagerSubsystem.h"
#include "Dialogue/UKDialogueSubsystem.h"
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

	// 우선순위:
	// 1) QuestTitle
	// 2) Title
	if ( !Def->QuestTitle.IsEmpty() )
	{
		return Def->QuestTitle;
	}

	return Def->Title;
}

FText UUKQuestUIManagerSubsystem::GetQuestDescriptionText(FName QuestId) const
{
	const UUKQuestDefinitionAsset* Def = GetQuestDefinitionSafe(QuestId);
	if ( !Def )
	{
		return FText::GetEmpty();
	}

	// 우선순위:
	// 1) QuestDescription
	// 2) Description
	if ( !Def->QuestDescription.IsEmpty() )
	{
		return Def->QuestDescription;
	}

	return Def->Description;
}

FText UUKQuestUIManagerSubsystem::GetQuestNPCDialogueText(FName QuestId) const
{
	const UUKQuestDefinitionAsset* Def = GetQuestDefinitionSafe(QuestId);
	if ( !Def )
	{
		return FText::GetEmpty();
	}

	return Def->NPCDialogue;
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

	return QS->CanStartQuestBySequence(QuestId);
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