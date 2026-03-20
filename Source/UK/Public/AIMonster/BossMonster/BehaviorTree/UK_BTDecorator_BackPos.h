#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UK_BTDecorator_BackPos.generated.h"

UCLASS()
class UK_API UUK_BTDecorator_BackPos : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UUK_BTDecorator_BackPos();
	
protected:
	UPROPERTY(EditAnywhere, Category="Range")
	float MaxDistance = 5000.0f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const override;
};
