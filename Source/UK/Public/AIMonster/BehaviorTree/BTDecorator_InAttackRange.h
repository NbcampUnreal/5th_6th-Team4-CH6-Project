#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_InAttackRange.generated.h"

UCLASS()
class UK_API UBTDecorator_InAttackRange : public UBTDecorator
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UBTDecorator_InAttackRange();
#pragma endregion

#pragma region Condition Check
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
#pragma endregion
};