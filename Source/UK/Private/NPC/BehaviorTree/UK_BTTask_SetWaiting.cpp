#include "NPC/BehaviorTree/UK_BTTask_SetWaiting.h"
#include "AIController.h"
#include "NPC/UK_PatrolNPC.h"

UUK_BTTask_SetWaiting::UUK_BTTask_SetWaiting()
{
	NodeName = "Set Waiting";
	bWaiting = true;
}

EBTNodeResult::Type UUK_BTTask_SetWaiting::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return EBTNodeResult::Failed;

	AUK_PatrolNPC* NPC =
		Cast<AUK_PatrolNPC>(AICon->GetPawn());

	if (!NPC)
		return EBTNodeResult::Failed;

	NPC->bIsWaiting = bWaiting;
	UE_LOG(LogTemp, Warning, TEXT("Waiting: %d"), bWaiting);
	return EBTNodeResult::Succeeded;
}