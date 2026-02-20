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
	HeavyAttackOnGround,
	DropAttack,
	NomalSkill,
	UltimateSkill
};
UCLASS()
class UK_API UUK_StatusAnimData : public UDataAsset
{
	GENERATED_BODY()
public:
	UUK_AnimData* FindAnimsDataAssetByType(const EComboAttackType AttackType);
	TObjectPtr<USkeletalMesh> GetLeftHandWeapon() const { return LeftHandWeapon; }
	TObjectPtr<USkeletalMesh> GetRightHandWeapon() const { return RightHandWeapon; }

	FVector GetLeftLocationOffset()const { return LeftLocationOffset; }
	FRotator GetLeftRotationOffset()const { return LeftRotationOffset; }

	FVector GetRightLocationOffset()const { return RightLocationOffset; }
	FRotator GetRightRotationOffset()const { return RightRotationOffset; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ComboAttack")
	TMap<EComboAttackType, TObjectPtr<UUK_AnimData>> ComboAnimationDatas;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<USkeletalMesh> LeftHandWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<USkeletalMesh> RightHandWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector LeftLocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FRotator LeftRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector RightLocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FRotator RightRotationOffset;
};
