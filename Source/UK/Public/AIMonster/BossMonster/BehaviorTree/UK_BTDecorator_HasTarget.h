#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UK_BTDecorator_HasTarget.generated.h"


UCLASS()
class UK_API UUK_BTDecorator_HasTarget : public UBTDecorator
{
	GENERATED_BODY()
public:
	UUK_BTDecorator_HasTarget();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const override;
};
