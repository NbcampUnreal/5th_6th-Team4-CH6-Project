#include "AIMonster/BehaviorTree/UK_BTTask_FindPatrolLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_FindPatrolLocation::UUK_BTTask_FindPatrolLocation()
{
	NodeName = "Find Patrol Location";

	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindPatrolLocation, PatrolLocationKey));
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindPatrolLocation, SpawnLocationKey));
}
#pragma endregion

#pragma region Patrol Location Search
EBTNodeResult::Type UUK_BTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	const FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);

	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
	{
		PatrolRadius = Monster->PatrolRadius;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ResultLocation;
		if (NavSystem->GetRandomPointInNavigableRadius(SpawnLocation, PatrolRadius, ResultLocation))
		{
			BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, ResultLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
#pragma endregion