// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "UK_DamageExecutionCalculation.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_DamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UUK_DamageExecutionCalculation();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
};
