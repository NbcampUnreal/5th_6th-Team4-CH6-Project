#include "UI/InGame/Quest/UK_QuestMain.h"

#include "UI/InGame/Quest/UK_QuestLists.h"
#include "Dialogue/UKQuestUIManagerSubsystem.h"


void UUK_QuestMain::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>();
	}

	if (!QuestLists || !QuestUIManager)
	{
		return;
	}

	for (const FName& QuestId : QuestIds)
	{
		const FText QuestTitle = QuestUIManager->GetQuestTitleText(QuestId);
		const FText QuestDescription = QuestUIManager->GetQuestDescriptionText(QuestId);

		QuestLists->AddQuestItem(QuestTitle, QuestDescription);
	}
}
