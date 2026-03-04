#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_HasTarget.h"
#include "BehaviorTree/BlackboardComponent.h"

UUK_BTDecorator_HasTarget::UUK_BTDecorator_HasTarget()
{
	NodeName = TEXT("Has Target");
}

bool UUK_BTDecorator_HasTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	UObject* Target = BB->GetValueAsObject(TEXT("TargetActor"));

	return IsValid(Target);
}
