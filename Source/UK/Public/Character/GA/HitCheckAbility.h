// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HitCheckAbility.generated.h"

/**
 * 
 */
class AAIMonsterBase;
UCLASS()
class UK_API UHitCheckAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	void Attack(FGameplayAbilityTargetDataHandle TargetDataHandle);
	
	//UFUNCTION(BlueprintCallable)
	//FName FindRowNameByAttackType();

	UPROPERTY()
	TArray<AAIMonsterBase*> HitcheckedActor;
};
