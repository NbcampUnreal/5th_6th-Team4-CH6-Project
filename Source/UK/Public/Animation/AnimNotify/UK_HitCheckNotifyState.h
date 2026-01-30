// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "UK_HitCheckNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_HitCheckNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "DamageInfo")
	TSubclassOf<class UDamageType> AttackDamageType;
};
