// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIMonster/UK_MonsterTypes.h"
#include "UK_GetHitAbility.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_GetHitAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetHitMontage();
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EHitReactionType HitType;
};
