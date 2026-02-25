// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "UK_TargetActorTrace.generated.h"

#define ECC_ATTACK ECollisionChannel::ECC_GameTraceChannel1

/**
 * 
 */
UCLASS()
class UK_API AUK_TargetActorTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	AUK_TargetActorTrace();

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

	static int32 ShowAttackDebug;
protected:
	UFUNCTION()
	TArray<TWeakObjectPtr<AActor>> GetTraceResult(AActor* InSourceActor);
	UPROPERTY(EditAnywhere)
	FName TraceStartSocketName;
	UPROPERTY(EditAnywhere)
	FName TraceEndSocketName;


};
