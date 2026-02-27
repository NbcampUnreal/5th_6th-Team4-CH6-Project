#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsAggressive.generated.h"

/**
 * 몬스터가 공격받아서 적대적인지 체크하는 Decorator
 */
UCLASS()
class UK_API UBTDecorator_IsAggressive : public UBTDecorator
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UBTDecorator_IsAggressive();
#pragma endregion

#pragma region Condition Check
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
#pragma endregion

#pragma region Observer
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion
};