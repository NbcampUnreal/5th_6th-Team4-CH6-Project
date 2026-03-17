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
		UAnimMontage* HitAnim = Player->HitMontageDataAsset->FindHitMontageByType(HitType);
		if (IsValid(HitAnim))
		{
			return HitAnim;
		}
	}
	return nullptr;
}

void UUK_GetHitAbility::CancelAbilitiesWithTags(const FGameplayTagContainer& Tags)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.IsActive() || !Spec.Ability) 
			continue;
		if (&Spec == GetCurrentAbilitySpec()) 
			continue; // 자기 자신 제외
		if (Spec.Ability->GetAssetTags().HasAny(Tags))
		{
			ASC->CancelAbilityHandle(Spec.Handle);
		}
	}
}