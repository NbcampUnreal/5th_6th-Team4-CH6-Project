// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "HitCheckAbility.generated.h"

/**
 * 
 */

UCLASS()
class UK_API UHitCheckAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UFUNCTION(blueprintCallable)
	void HitPlaySound(FGameplayAbilityTargetDataHandle Targets);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EComboAttackType AttackType;
};
