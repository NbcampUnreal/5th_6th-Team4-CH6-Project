#include "AIMonster/BehaviorTree/BTDecorator_InAttackRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/Monster/UK_EliteMonster.h"

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

	// 스페셜 어택 중에는 어보트 방지
	if (AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(Pawn))
	{
		if (Elite->bIsSpecialAttacking) return true;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject("TargetPlayer"));
	if (!Target) return false;

	const FVector MonsterLoc = Pawn->GetActorLocation();
	const FVector TargetLoc  = Target->GetActorLocation();
	const float   Dist       = FVector::Dist(MonsterLoc, TargetLoc);

	float AttackRange = 200.f;
	float AttackAngle = 150.f; // 전방 ±75
	if (FVector::DistSquared(MonsterLoc, TargetLoc) > FMath::Square(AttackRange)) return false;

	if (Dist > AttackRange) return false;

	// 전방 각도 체크: 몬스터 정면 기준 AttackAngle 이내에 있어야 공격 가능
	const FVector RawForward = Pawn->GetActorForwardVector();
	const FVector Forward2D  = FVector(RawForward.X, RawForward.Y, 0.f).GetSafeNormal();
	const FVector ToTarget   = (TargetLoc - MonsterLoc).GetSafeNormal2D();
	const float   DotProduct = FVector::DotProduct(Forward2D, ToTarget);
	const float   CosHalf    = FMath::Cos(FMath::DegreesToRadians(AttackAngle * 0.5f));
 
	return DotProduct >= CosHalf;
}
#pragma endregion