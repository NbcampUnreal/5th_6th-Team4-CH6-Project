// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/UK_CharacterBase.h"
#include "UK_WeaponItemData.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)

struct FUK_WeaponItemData : public FTableRowBase
{
GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ItemStackCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MaxItemStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ItemTag;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ECharacterAttribute WeaponAttribute;
};
