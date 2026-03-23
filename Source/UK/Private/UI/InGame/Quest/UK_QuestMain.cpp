#include "UI/InGame/Quest/UK_QuestMain.h"

#include "UI/InGame/Quest/UK_QuestLists.h"
#include "Engine/GameInstance.h"
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include <Kismet/GameplayStatics.h>
#include <Systems/UK_GameInstance.h>
#include "Components/ScrollBox.h" 


void UUK_QuestMain::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>();
		QuestManager = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	}

	RefreshQuestList();
}

void UUK_QuestMain::RefreshQuestList()
{
	if (!QuestLists || !QuestUIManager || !QuestManager) return;

	QuestLists->QuestScrollBox->ClearChildren();

	for (const TPair<FName, FQuestProgress>& Pair : QuestManager->RuntimeProgress)
	{
		const FName QuestId = Pair.Key;
		const FText QuestTitle = QuestUIManager->GetQuestTitleText(QuestId);
		const FText QuestDescription = QuestUIManager->GetQuestDescriptionText(QuestId);

		QuestLists->AddQuestItem(QuestTitle, QuestDescription);
	}
}
