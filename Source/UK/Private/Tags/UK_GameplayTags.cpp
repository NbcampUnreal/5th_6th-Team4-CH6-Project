// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/UK_GameplayTags.h"

namespace UK_GameplayTags
{
	namespace Input
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Move, "Input.Move", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Look, "Input.Look", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Jump, "Input.Jump", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sprint, "Input.Sprint", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ZoomIn, "Input.Zoom.In", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ZoomOut, "Input.Zoom.Out", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(NomalSkill, "Input.NomalSkill", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(UltimateSkill, "Input.UltimateSkill", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Crouch, "Input.Crouch", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ToggleMouse, "Input.Default.ToggleMouse", "Default Movement Ability")
		//UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction, "Input.Default.Interaction", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Setting, "Input.Default.Setting", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction, "Input.Interaction", "Default Movement Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(LockOnToggle, "Input.LockOnToggle", "Default Movement Ability")
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
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(LightAttack, "Action.LightAttack", "LightAttack Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(HeavyAttack, "Action.HeavyAttack", "HeavyAttack Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(AirAttack, "Action.AirAttack", "AirAttack Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(DropAttack, "Action.DropAttack", "DropAttack Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnGroundAttack, "Action.OnGroundAttack", "OnGroundAttack Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Parry, "Action.Parry", "Parry Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Parrying, "Action.Parrying", "Parrying Ability")
	}

	namespace Attack
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attacking, "Attack.Attacking", "Attacking Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Next, "Attack.Next", "Next Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitCheck, "Attack.HitCheck", "HitCheck Ability")
	}
	namespace Status
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dead, "Status.Dead", "PlayerStatus")
	}
	namespace Data
	{
		namespace Damages
		{
			namespace OneHandSword
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Light1, "Data.Damages.OneHandSword.Light1", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Light2, "Data.Damages.OneHandSword.Light2", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Light3, "Data.Damages.OneHandSword.Light3", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Light4, "Data.Damages.OneHandSword.Light4", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Heavy1, "Data.Damages.OneHandSword.Heavy1", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Heavy2, "Data.Damages.OneHandSword.Heavy2", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Heavy3, "Data.Damages.OneHandSword.Heavy3", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Heavy4, "Data.Damages.OneHandSword.Heavy4", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Air1, "Data.Damages.OneHandSword.Air1", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Air2, "Data.Damages.OneHandSword.Air2", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Air3, "Data.Damages.OneHandSword.Air3", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Air4, "Data.Damages.OneHandSword.Air4", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Drop, "Data.Damages.OneHandSword.Drop", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(NormalSkill, "Data.Damages.OneHandSword.NormalSkill", "DamageTag")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(UltimateSkill, "Data.Damages.OneHandSword.UltimateSkill", "DamageTag")
			}
		}
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage, "Data.Damage", "Damage Data")
		
	}

	namespace CoolDown
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(NomalSkill, "CoolDown.NomalSkill", "Nomalskill CoolDown")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(UltimateSkill, "CoolDown.UltimateSkill", "UltimateSkill CoolDown")
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
