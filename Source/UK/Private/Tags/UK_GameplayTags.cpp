// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/UK_GameplayTags.h"

namespace UK_GameplayTags
{
	namespace Input
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Move, "Input.Default.Move", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Look, "Input.Default.Look", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Jump, "Input.Default.Jump", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sprint, "Input.Default.Sprint", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ZoomIn, "Input.Default.Zoom.In", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ZoomOut, "Input.Default.Zoom.Out", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(LightAttack, "Input.Default.LightAttack", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HeavyAttack, "Input.Default.HeavyAttack", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(NomalSkill, "Input.Default.NomalSkill", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(UltimateSkill, "Input.Default.UltimateSkill", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Crouch, "Input.Default.Crouch", "Default Movement Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ToggleMouse, "Input.Default.ToggleMouse", "Default Movement Ability")
	}
	namespace Weapon
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(WeaponRoot, "Weapon", "Root Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(DefaultWeapon, "Weapon.Default", "Default Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dagger, "Weapon.Dagger", "Dagger Weapon Ability")
	}
	namespace Action
	{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Swap1, "Action.Swap1", "Swap Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Swap2, "Action.Swap2", "Swap Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Swap3, "Action.Swap3", "Swap Weapon Ability")

	}
	namespace Attack
	{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attacking, "Attack.Attacking", "Attacking Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Next, "Attack.Next", "Attacking Ability")
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