// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UK_AnimData.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_AnimData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> ComboMantage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ComboData")
	FString MontageName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ComboData")
	uint8 MaxComboCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ComboData")
	TArray<float> ComboFrameTime;
};
