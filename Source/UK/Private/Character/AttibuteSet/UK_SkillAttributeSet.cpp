// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AttibuteSet/UK_SkillAttributeSet.h"

UUK_SkillAttributeSet::UUK_SkillAttributeSet() :
	Cost(0.f),
	CoolDown(0.f)
{

}

void UUK_SkillAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
}

void UUK_SkillAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	if ( Attribute == GetCostAttribute() )
	{
		OnCostChanged.Broadcast(OldValue, NewValue);
	}
	if ( Attribute == GetCoolDownAttribute() )
	{
		OnCostChanged.Broadcast(OldValue, NewValue);
	}
}
