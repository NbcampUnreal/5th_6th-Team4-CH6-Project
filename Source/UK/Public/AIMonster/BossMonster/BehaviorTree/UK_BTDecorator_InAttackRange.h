#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UK_BTDecorator_InAttackRange.generated.h"

UCLASS()
class UK_API UUK_BTDecorator_InAttackRange : public UBTDecorator
{
	GENERATED_BODY()
public:
	UUK_BTDecorator_InAttackRange();

protected:
	UPROPERTY(EditAnywhere, Category="AI")
	float AttackRange = 200.f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const override;
};
