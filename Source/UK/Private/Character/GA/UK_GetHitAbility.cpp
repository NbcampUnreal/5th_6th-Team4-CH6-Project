// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GA/UK_GetHitAbility.h"
#include "Character/UK_CharacterBase.h"

UAnimMontage* UUK_GetHitAbility::GetHitMontage()
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	if (IsValid(Player) == false)
		return nullptr;
		
	if (IsValid(Player->HitMontageDataAsset))
	{
		return Player->HitMontageDataAsset->FindHitMontageByType(HitType);
		
	}
	return nullptr;
}
