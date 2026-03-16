// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/UK_CharacterBase.h"
#include "UK_NPCData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FUK_NPCData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName NPCID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText NPCName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText NPCDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag NPCTag;
	
};