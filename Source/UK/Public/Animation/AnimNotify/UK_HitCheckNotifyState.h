// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "UK_HitCheckNotifyState.generated.h"

/**
 * 
 */
class AUK_CharacterBase;

UCLASS()
class UK_API UUK_HitCheckNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference )override;

protected:

	void HitCheck();
	FTimerHandle HitTimer;
				
	AUK_CharacterBase* OwnerCharacter;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag AttackTag;

};
