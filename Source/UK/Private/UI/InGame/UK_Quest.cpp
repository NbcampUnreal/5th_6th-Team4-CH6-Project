#include "UI/InGame/UK_Quest.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"

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
	UE_LOG(LogTemp, Log, TEXT("[QuestUI] Accept Clicked"));

	if (CurrentQuestId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestUI] QuestId is None"));
		return;
	}

	UUKQuestManagerSubsystem* QuestSys = GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>();

	if (QuestSys)
	{
		const FString EventStr = FString::Printf(TEXT("QuestEvent.Accepted.%s"), *CurrentQuestId.ToString());

		QuestSys->EmitQuestEvent(FName(*EventStr));
	}

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(GetOwningPlayer());

	if (PlayerCtl)
	{
		PlayerCtl->ApplyInputState(EInputState::Game);
		PlayerCtl->SetCursorVisible(false);
	}

	RemoveFromParent();
}

void UUK_Quest::OnExitButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[QuestUI] Exit Clicked"));
	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(GetOwningPlayer());
	if (PlayerCtl)
	{
		PlayerCtl->ApplyInputState(EInputState::Game);
		PlayerCtl->SetCursorVisible(false);
	}

	RemoveFromParent();
}

