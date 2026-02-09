// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "UK_StatusAnimData.generated.h"

#pragma region Forward Declaration
class UUK_AnimData;
class AUK_WeaponBase;
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
	UUK_AnimData* FindAnimsDataAssetByType(const EComboAttackType AttackType);
	TSubclassOf<AUK_WeaponBase> GetWeapon() { return Weapon; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponMesh")
	TSubclassOf<AUK_WeaponBase> Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ComboAttack")
	TMap<EComboAttackType, TObjectPtr<UUK_AnimData>> ComboAnimationDatas;
};
