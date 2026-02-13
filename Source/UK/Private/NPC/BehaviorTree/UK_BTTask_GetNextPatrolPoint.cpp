#include "NPC/BehaviorTree/UK_BTTask_GetNextPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC/UK_PatrolNPC.h"

UUK_BTTask_GetNextPatrolPoint::UUK_BTTask_GetNextPatrolPoint()
{
	NodeName = TEXT("Get Next Patrol Point");
}

EBTNodeResult::Type
UUK_BTTask_GetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory)
{
	AAIController* AICon =
		OwnerComp.GetAIOwner();

	if (!AICon)
		return EBTNodeResult::Failed;

	AUK_PatrolNPC* NPC =
		Cast<AUK_PatrolNPC>(AICon->GetPawn());

	if (!NPC)
		return EBTNodeResult::Failed;

	FVector Target = NPC->GetNextPatrolPoint();

	UBlackboardComponent* BB =
		OwnerComp.GetBlackboardComponent();

	if ( !BB )
		return EBTNodeResult::Failed;

	BB->SetValueAsVector(
		TEXT("PatrolTarget"),
		Target
	);

	return EBTNodeResult::Succeeded;
}