#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_InAttackRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UUK_BTDecorator_InAttackRange::UUK_BTDecorator_InAttackRange()
{
	NodeName = TEXT("In Attack Range");
}

bool UUK_BTDecorator_InAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	AActor* Target =
		Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));

	if (!Target) return false;

	auto* AI = OwnerComp.GetAIOwner();
	if (!AI) return false;

	APawn* Pawn = AI->GetPawn();
	if (!Pawn) return false;

	float Dist = FVector::Dist(
		Pawn->GetActorLocation(),
		Target->GetActorLocation()
	);

	return Dist <= AttackRange;
}
