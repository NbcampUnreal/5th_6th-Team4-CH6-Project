// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AIMonster/UK_MonsterTypes.h"
#include "HitMontageDataAsset.generated.h"


/**
 * 
 */
UCLASS()
class UK_API UHitMontageDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION()
	UAnimMontage* FindHitMontageByType(EHitReactionType TargetType);
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TMap<EHitReactionType,TObjectPtr<UAnimMontage>> HitMontageMap;
};
