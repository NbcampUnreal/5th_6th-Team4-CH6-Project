// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UK_CharacterTypes.generated.h"

UENUM(BlueprintType)
enum class EInputMode : uint8
{
	None,
	Light,
	Heavy,
	Air,
	NormalSkill,
	UltimateSkill,
	Parry,
	Dash
};

UENUM(BlueprintType)
enum class ECharacterAttribute : uint8
{
	None,
	Fire,
	Wind
};
UENUM()
enum class EComboAttackType : uint8
{
	None,
	AttackOnAir,
	LightAttackOnGround,
	HeavyAttackOnGround,
	DropAttack,
	NomalSkill,
	UltimateSkill,
	Parry,
	Dash
};
UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None UMETA(DisplayName="None"),
	CMOVE_Climb UMETA(DisplayName="Climb"),
	CMOVE_Gliding UMETA(DisplayName="Gliding")
};