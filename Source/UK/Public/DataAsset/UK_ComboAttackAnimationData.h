// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UK_ComboAttackAnimationData.generated.h"

UENUM()
enum class ECharacterAttackType : uint8
{
	None,
	AttackOnAir,
	LightAttackOnGround,
	HeavyAttackOnGround,
};

UCLASS()
class UK_API UUK_ComboAttackAnimationData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ComboAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ComboData")
	FString MontageSectionNamePrefix;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ComboData")
	uint8 MaxComboCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ComboData")
	TArray<float> ComboAcceptableFrameTime;

	
};
