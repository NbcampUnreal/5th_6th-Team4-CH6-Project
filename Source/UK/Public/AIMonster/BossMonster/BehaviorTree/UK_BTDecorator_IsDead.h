// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UK_BTDecorator_IsDead.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_BTDecorator_IsDead : public UBTDecorator
{
	GENERATED_BODY()
public:
	UUK_BTDecorator_IsDead();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const override;
};
