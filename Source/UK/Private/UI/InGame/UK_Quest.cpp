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

void UUK_Quest::OnPlayButtonClicked()
{

	if (CurrentQuestId.IsNone()) return;


	UUKQuestManagerSubsystem* QuestSys = GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>();

	if (QuestSys)
	{
		const FString EventStr = FString::Printf(TEXT("QuestEvent.Accepted.%s"), *CurrentQuestId.ToString());
		QuestSys->EmitQuestEvent(FName(*EventStr));
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
	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(GetOwningPlayer());
	if ( PlayerCtl )
	{
		PlayerCtl->ApplyInputState(EInputState::Game);
		PlayerCtl->SetCursorVisible(false);
	}

	RemoveFromParent();
}

