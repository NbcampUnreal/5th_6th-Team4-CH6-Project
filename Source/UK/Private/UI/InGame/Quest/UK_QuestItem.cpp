#include "UI/InGame/Quest/UK_QuestItem.h"
#include "Components/TextBlock.h"

void UUK_QuestItem::SetQusetData(const FText& InQuestTitle, const FText& InQuestDescription)
{
	if (QuestTitleText)
	{
		QuestTitleText->SetText(InQuestTitle);
	}
	if (QuestDescriptionText)
	{
		QuestDescriptionText->SetText(InQuestDescription);
	}
}
