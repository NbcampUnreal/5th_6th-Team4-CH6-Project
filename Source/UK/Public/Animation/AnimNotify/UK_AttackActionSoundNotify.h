// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UK_AttackActionSoundNotify.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_AttackActionSoundNotify : public UAnimNotify
{
	GENERATED_BODY()
private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound | Data")
	TObjectPtr<USoundBase> AttackSound;

};
