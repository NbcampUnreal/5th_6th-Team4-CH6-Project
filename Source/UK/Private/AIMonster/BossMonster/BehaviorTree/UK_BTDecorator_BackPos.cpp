#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_BackPos.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UUK_BTDecorator_BackPos::UUK_BTDecorator_BackPos()
{
	NodeName = TEXT("Back Home Pos");
	bNotifyTick = true;
	FlowAbortMode = EBTFlowAbortMode::Both;
}

bool UUK_BTDecorator_BackPos::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	auto* AI = OwnerComp.GetAIOwner();
	if (!AI) return false;

	APawn* Pawn = AI->GetPawn();
	if (!Pawn) return false;

	FVector Home = BB->GetValueAsVector(TEXT("HomeLocation"));

	float Dist = FVector::Dist(
		Pawn->GetActorLocation(),
		Home
	);
	UE_LOG(LogTemp, Warning,
		TEXT("Dist: %.1f / Max: %.1f"),
		Dist,
		MaxDistance
	);
	return Dist > MaxDistance;
}