// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/UK_GameplayTags.h"

namespace UK_GameplayTags
{
	namespace Input
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputMove, "Input.Default.Move", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputLook, "Input.Default.Look", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputJump, "Input.Default.Jump", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputSprint, "Input.Default.Sprint", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputZoomIn, "Input.Default.Zoom.In", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputZoomOut, "Input.Default.Zoom.Out", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputAttack, "Input.Default.Attack", "Default Movement Ability")
	}
	namespace Weapon
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(WeaponRoot, "Weapon", "Root Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(DefaultWeapon, "Weapon.Default", "Default Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dagger, "Weapon.Dagger", "Dagger Weapon Ability")
	}
	namespace Food
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(FoodRoot, "Food", "Root Food Tag")
	}
	namespace Material
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaterialRoot, "Material", "Root Material Tag")
	}
}