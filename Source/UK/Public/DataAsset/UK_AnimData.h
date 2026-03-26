// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/UK_CharacterTypes.h"
#include "UK_AnimData.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_AnimData : public UDataAsset
{
	GENERATED_BODY()
public:
	
	UAnimMontage* FindMontageByAttribute(ECharacterAttribute Attribute);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation | Montage")
	TObjectPtr<UAnimMontage> ComboMantage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation | Montage")
	TMap<ECharacterAttribute, UAnimMontage*> ComboMontageMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation | Data")
	TObjectPtr<UBlendSpace> BlendSpace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation | Data")
	FString MontageSectionName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound | Data")
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound | Data")
	TObjectPtr<USoundBase> HitSound;

};
