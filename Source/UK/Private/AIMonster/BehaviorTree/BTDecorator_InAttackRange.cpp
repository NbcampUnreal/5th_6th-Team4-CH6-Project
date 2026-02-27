#include "AIMonster/BehaviorTree/BTDecorator_InAttackRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UBTDecorator_InAttackRange::UBTDecorator_InAttackRange()
{
	NodeName = "In Attack Range";
	FlowAbortMode = EBTFlowAbortMode::Both;
}
#pragma endregion

#pragma region Condition Check
bool UBTDecorator_InAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return false;

	APawn* Pawn = AI->GetPawn();
	if (!Pawn) return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject("TargetPlayer"));
	if (!Target) return false;

	const float Dist = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());

	float AttackRange = 200.f;
	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn))
	{
		AttackRange = Monster->AttackRange;
	}

	return Dist <= AttackRange;
}
#pragma endregion