// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitMontageDataAsset.generated.h"


UENUM()
enum class EHitType : uint8
{
	None,
	Normal
};
/**
 * 
 */
UCLASS()
class UK_API UHitMontageDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION()
	UAnimMontage* FindHitMontageByType(EHitType TargetType);
protected:
	UPROPERTY()
	TMap<EHitType,UAnimMontage*> HitMontageMap;
};
