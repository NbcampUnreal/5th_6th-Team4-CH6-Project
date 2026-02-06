// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UK_WeaponData.generated.h"

#pragma region Forward Declaration
class UUK_StatusAnimData;
#pragma endregion

UCLASS()
class UK_API UUK_WeaponData : public UDataAsset
{
	GENERATED_BODY()
public:
	UUK_StatusAnimData* FindAnimsDataAssetByTag(const FGameplayTag& WeaponTag);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponCombo")
	TMap<FGameplayTag, TObjectPtr <UUK_StatusAnimData>> WeaponAnimationDatas;

};
