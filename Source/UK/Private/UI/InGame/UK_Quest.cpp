#include "UI/InGame/UK_Quest.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "Character/UK_PlayerController.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "UI/InGame/Quest/UK_QuestMain.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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

void UUK_Quest::SetQuestUI(FName QuestID,const FText& NPCName,const FText& Dialogue,const FText& QuestDesc,const FText& AcceptText,const FText& ExitText)
{
	CurrentQuestId = QuestID;

	if (NPCNameText)
		NPCNameText->SetText(NPCName);

	if  (DialogueText)
		DialogueText->SetText(Dialogue);

	if (QuestDescText)
		QuestDescText->SetText(QuestDesc);

	if (AcceptTextBlock)
		AcceptTextBlock->SetText(AcceptText);

	if (ExitTextBlock)
		ExitTextBlock->SetText(ExitText);
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

	const FText SpeakerName = QuestUIManager->GetCurrentDialogueSpeakerName();
	const FText CurrentDialogue = QuestUIManager->GetCurrentDialogueText();

	if ( NPCNameText )
	{
		NPCNameText->SetText(SpeakerName);
	}

	if ( DialogueText )
	{
		DialogueText->SetText(CurrentDialogue);
	}

	FText CurrentChoiceText = FText::GetEmpty();
	const TArray<FText> ChoiceTexts = QuestUIManager->GetCurrentDialogueChoiceTexts();
	if ( ChoiceTexts.Num() > 0 )
	{
		CurrentChoiceText = ChoiceTexts[ 0 ];
	}

	if ( AcceptTextBlock )
	{
		AcceptTextBlock->SetText(CurrentChoiceText);
	}

	if ( Accept_Button )
	{
		Accept_Button->SetVisibility(ChoiceTexts.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
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

	// 선택 후 현재 대화 상태 확인
	const FText CurrentDialogue = QuestUIManager->GetCurrentDialogueText();
	const TArray<FText> ChoiceTexts = QuestUIManager->GetCurrentDialogueChoiceTexts();

	const bool bDialogueEnded = CurrentDialogue.IsEmpty() && ChoiceTexts.Num() == 0;

	if ( !bDialogueEnded )
	{
		// 아직 다음 노드가 남아 있으면 UI만 갱신하고 유지
		RefreshDialogueUI();
		return;
	}

	// 대화가 완전히 끝났을 때만 퀘스트 목록 갱신 + UI 닫기
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
	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(GetOwningPlayer());
	if ( PlayerCtl )
	{
		PlayerCtl->ApplyInputState(EInputState::Game);
		PlayerCtl->SetCursorVisible(false);
	}

	RemoveFromParent();
}

