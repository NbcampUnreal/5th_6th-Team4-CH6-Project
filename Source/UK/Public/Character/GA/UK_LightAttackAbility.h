// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "UK_LightAttackAbility.generated.h"

/**
 * 
 */


UCLASS()
class UK_API UUK_LightAttackAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	UUK_LightAttackAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable)
	UAnimMontage* StartCombo(EComboAttackType InStatus);

	UFUNCTION(BlueprintCallable)
	void ResetInput();

	UFUNCTION(BlueprintCallable)
	void CharactorStopJump();
	UFUNCTION(BlueprintCallable)
	void ResetEndCombo();

	UFUNCTION(BlueprintNativeEvent)
	void EndCombo();


protected:
	
	UPROPERTY(BlueprintReadWrite)
	int32 index;

	UPROPERTY(BlueprintReadWrite)
	FString SectionString;
};
