// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace UK_GameplayTags
{
	namespace Input
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputMove);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputLook);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputJump);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSprint);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAttack);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputZoomIn);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputZoomOut);
	}
	namespace Weapon
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(WeaponRoot);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(DefaultWeapon);
	}
}