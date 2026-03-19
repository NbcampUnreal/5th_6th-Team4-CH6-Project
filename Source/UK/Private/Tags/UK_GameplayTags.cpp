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
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dash, "Input.Dash", "Default Movement Ability")
	}

	namespace Movement
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(None, "Movement.None", "Movement")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Walking, "Movement.Walking", "Movement")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Swimming, "Movement.Swimming", "Movement")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gliding, "Movement.Gliding", "Movement")
	}

	namespace Weapon
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(WeaponRoot, "Weapon", "Root Weapon Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(DefaultWeapon, "Weapon.Default", "Default Weapon Ability")

		namespace OndHandSword
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(BaseSword, "Weapon.OndHandSword.BaseSword", "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(CanineSword, "Weapon.OndHandSword.CanineSword",
			                               "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ClawSword, "Weapon.OndHandSword.ClawSword", "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(BlueStoneSword, "Weapon.OndHandSword.BlueStoneSword",
			                               "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(WolfSword, "Weapon.OndHandSword.WolfSword", "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(StoneSword, "Weapon.OndHandSword.StoneSword", "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScorpionSword, "Weapon.OndHandSword.ScorpionSword",
			                               "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(PoisonSword, "Weapon.OndHandSword.PoisonSword",
			                               "OndHandSword Weapon Ability")
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(GruxSword, "Weapon.OndHandSword.GruxSword", "OndHandSword Weapon Ability")
		}
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
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Glide, "Action.Glide", "Glide Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Swim, "Action.Swim", "Swim Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(StopSwimming, "Action.StopSwimming", "StopSwimming Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(invincibility, "Action.invincibility", "invincibility Ability")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(BeAttacked, "Action.BeAttacked", "BeAttacked Ability")
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

		namespace Cost
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(SkillCost, "Data.Cost.SkillCost", "Cost Data")

			namespace OneHandSword
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(NormalSkill, "Data.Cost.OneHandSword.NormalSkill", "Cost Data")
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(UltimateSkill, "Data.Cost.OneHandSword.UltimateSkill", "Cost Data")
			}
		}

		namespace WeaponStat
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ExtraAttackPower, "Data.WeaponStat.ExtraAttackPower", "WeaponStat")
		}

		namespace EndBattle
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Heal, "Data.EndBattle.Heal", "EndBattle")
		}
	}

	namespace CoolDown
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(NomalSkill, "CoolDown.NomalSkill", "Nomalskill CoolDown")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(UltimateSkill, "CoolDown.UltimateSkill", "UltimateSkill CoolDown")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Hit, "CoolDown.Hit", "Hit CoolDown")
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
