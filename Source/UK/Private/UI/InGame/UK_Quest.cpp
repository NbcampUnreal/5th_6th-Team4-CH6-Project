#include "UI/InGame/UK_Quest.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "Character/UK_PlayerController.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "UI/InGame/Quest/UK_QuestMain.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Dialogue/UKDialogueSubsystem.h"

UUK_Quest::UUK_Quest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UUK_Quest::NativeConstruct()
{
	Super::NativeConstruct();

	if (Accept_Button)
	{
		Accept_Button->OnClicked.AddDynamic(this,&UUK_Quest::OnPlayButtonClicked);
	}
	if (Exit_Button)
	{
		Exit_Button->OnClicked.AddDynamic(this,&UUK_Quest::OnExitButtonClicked);
	}
}

void UUK_Quest::SetQuestUI(FName QuestID, const FText& NPCName, const FText& Dialogue, const FText& QuestDesc, const FText& AcceptText, const FText& ExitText)
{
	CurrentQuestId = QuestID;

	if ( NPCNameText )
	{
		NPCNameText->SetText(NPCName);
	}

	if ( DialogueText )
	{
		DialogueText->SetText(Dialogue);
	}

	if ( QuestDescText )
	{
		QuestDescText->SetText(QuestDesc);
	}

	if ( AcceptTextBlock )
	{
		AcceptTextBlock->SetText(AcceptText);
	}

	if ( Accept_Button )
	{
		Accept_Button->SetVisibility(AcceptText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if ( ExitTextBlock )
	{
		ExitTextBlock->SetText(ExitText);
	}
}

void UUK_Quest::SetDialogueOnly(const FText& NPCName, const FText& Dialogue)
{
	if ( NPCNameText )
	{
		NPCNameText->SetText(NPCName);
	}

	if ( DialogueText )
	{
		DialogueText->SetText(Dialogue);
	}
}

void UUK_Quest::RefreshDialogueUI()
{
	UGameInstance* GI = GetGameInstance();
	if ( !GI )
	{
		return;
	}

	UUKQuestUIManagerSubsystem* QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>();
	if ( !QuestUIManager )
	{
		return;
	}

	const FUKCurrentDialogueUIData UIData = QuestUIManager->GetCurrentDialogueUIData();

	if ( NPCNameText && !UIData.SpeakerName.IsEmpty() )
	{
		NPCNameText->SetText(UIData.SpeakerName);
	}

	if ( DialogueText && !UIData.DialogueText.IsEmpty() )
	{
		DialogueText->SetText(UIData.DialogueText);
	}

	FText CurrentChoiceText = FText::GetEmpty();
	if ( UIData.Choices.Num() > 0 )
	{
		CurrentChoiceText = UIData.Choices[ 0 ].ChoiceText;
	}

	if ( AcceptTextBlock )
	{
		AcceptTextBlock->SetText(CurrentChoiceText);
	}

	if ( Accept_Button )
	{
		Accept_Button->SetVisibility(UIData.Choices.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UUK_Quest::OnPlayButtonClicked()
{
	UGameInstance* GI = GetGameInstance();
	if ( !GI )
	{
		return;
	}

	UUKQuestUIManagerSubsystem* QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>();
	if ( !QuestUIManager )
	{
		return;
	}

	const bool bSelected = QuestUIManager->SelectDialogueChoice(0);
	if ( !bSelected )
	{
		return;
	}

	const FUKCurrentDialogueUIData UIData = QuestUIManager->GetCurrentDialogueUIData();

	// 아직 다음 노드가 남아 있으면 UI 갱신만 하고 유지
	if ( !UIData.bIsEnd && ( !UIData.DialogueText.IsEmpty() || UIData.Choices.Num() > 0 ) )
	{
		RefreshDialogueUI();
		return;
	}

	// 여기서부터는 실제 종료
	if ( UUKDialogueSubsystem* DialogueSys = GI->GetSubsystem<UUKDialogueSubsystem>() )
	{
		DialogueSys->EndDialogue();
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUK_QuestMain::StaticClass(), false);

	for ( UUserWidget* Widget : FoundWidgets )
	{
		if ( UUK_QuestMain* QuestMain = Cast<UUK_QuestMain>(Widget) )
		{
			QuestMain->RefreshQuestList();
		}
	}

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(GetOwningPlayer());
	if ( PlayerCtl )
	{
		PlayerCtl->ApplyInputState(EInputState::Game);
		PlayerCtl->SetCursorVisible(false);
	}

	RemoveFromParent();
}

void UUK_Quest::OnExitButtonClicked()
{
	if ( UGameInstance* GI = GetGameInstance() )
	{
		if ( UUKDialogueSubsystem* DialogueSys = GI->GetSubsystem<UUKDialogueSubsystem>() )
		{
			DialogueSys->EndDialogue();
		}
	}

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(GetOwningPlayer());
	if ( PlayerCtl )
	{
		PlayerCtl->ApplyInputState(EInputState::Game);
		PlayerCtl->SetCursorVisible(false);
	}

	RemoveFromParent();
}
