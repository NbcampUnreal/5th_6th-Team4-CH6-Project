// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace UK_GameplayTags
{
	namespace Input
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Move);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Look);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ZoomIn);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ZoomOut);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(LightAttack);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeavyAttack);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crouch);
	}
	namespace Weapon
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(WeaponRoot);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(DefaultWeapon);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dagger);
	}
	namespace Food
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(FoodRoot);
	}
	namespace Material
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaterialRoot);
	}
}