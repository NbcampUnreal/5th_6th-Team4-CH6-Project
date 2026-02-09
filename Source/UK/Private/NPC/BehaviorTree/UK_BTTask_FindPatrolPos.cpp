#include "NPC/BehaviorTree/UK_BTTask_FindPatrolPos.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UUK_BTTask_FindPatrolPos::UUK_BTTask_FindPatrolPos()
{
	NodeName = "Find Patrol Position";
}

EBTNodeResult::Type UUK_BTTask_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if ( !AI ) return EBTNodeResult::Failed;

	APawn* NPC = AI->GetPawn();
	if ( !NPC ) return EBTNodeResult::Failed;

	UNavigationSystemV1* Nav =
		UNavigationSystemV1::GetCurrent(NPC->GetWorld());

	if ( !Nav ) return EBTNodeResult::Failed;

	FNavLocation Result;

	if ( Nav->GetRandomReachablePointInRadius(
		NPC->GetActorLocation(),
		PatrolRadius,
		Result) )
	{
		OwnerComp.GetBlackboardComponent()->
			SetValueAsVector(PatrolLocationKey.SelectedKeyName, Result.Location);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}