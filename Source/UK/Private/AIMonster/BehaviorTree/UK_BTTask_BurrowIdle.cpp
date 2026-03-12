#include "AIMonster/BehaviorTree/UK_BTTask_BurrowIdle.h"
#include "AIController.h"
#include "AIMonster/Monster/UK_BurrowMonster.h"

#pragma region Initialization
UUK_BTTask_BurrowIdle::UUK_BTTask_BurrowIdle()
{
	NodeName    = "Burrow Idle";
	bNotifyTick = false;
}
#pragma endregion

#pragma region Task Interface
EBTNodeResult::Type UUK_BTTask_BurrowIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	AICon->StopMovement();

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UUK_BTTask_BurrowIdle::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::Aborted;
}

void UUK_BTTask_BurrowIdle::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
#pragma endregion