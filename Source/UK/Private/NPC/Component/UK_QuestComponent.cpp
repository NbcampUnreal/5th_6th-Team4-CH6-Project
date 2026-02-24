#include "NPC/Component/UK_QuestComponent.h"
#include "Net/UnrealNetwork.h"

UUK_QuestComponent::UUK_QuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UUK_QuestComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUK_QuestComponent::ProcessQuest(int32 QuestID, class AUK_QuestNPC* NPC)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_ProcessQuest(QuestID);
		return;
	}
	Server_ProcessQuest(QuestID);
}

void UUK_QuestComponent::Server_ProcessQuest_Implementation(int32 QuestID)
{
	for (auto& Quest : QuestList)
	{
		if (Quest.QuestID == QuestID)
		{
			Quest.bCompleted = true;
			return;
		}
	}

	FQuestData NewQuest;
	NewQuest.QuestID = QuestID;
	NewQuest.bCompleted = false;

	QuestList.Add(NewQuest);
}

void UUK_QuestComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UUK_QuestComponent, QuestList);
}

