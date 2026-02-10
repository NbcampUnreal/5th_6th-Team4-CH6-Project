
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_IsPlayerTooClose.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UBTDecorator_IsPlayerTooClose : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTDecorator_IsPlayerTooClose();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float CheckDistance = 200.f;
};
