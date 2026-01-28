// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UK_StatusAnimData.generated.h"

#pragma region Forward Declaration
class UUK_AnimData;
#pragma endregion
UENUM()
enum class EComboAttackType : uint8
{
	None,
	AttackOnAir,
	LightAttackOnGround,
	HeavyAttackOnGround
};
UCLASS()
class UK_API UUK_StatusAnimData : public UDataAsset
{
	GENERATED_BODY()
public:
	UUK_AnimData* FindAnimsDataAssetByTag(const EComboAttackType AttackType);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ComboAttack")
	TMap<EComboAttackType, TObjectPtr<UUK_AnimData>> ComboAnimationDatas;
};
