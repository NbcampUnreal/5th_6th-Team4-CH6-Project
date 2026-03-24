// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GA/HitCheckAbility.h"
#include "Character/UK_CharacterBase.h"
#include "Abilities/GameplayAbility.h"
#include "DataAsset/UK_AnimData.h"
#include "Kismet/GameplayStatics.h"


void UHitCheckAbility::HitPlaySound(FGameplayAbilityTargetDataHandle Targets)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	if (IsValid(Player) == false)
		return;

	if (FGameplayAbilityTargetData_ActorArray* ActorData =
		static_cast<FGameplayAbilityTargetData_ActorArray*>(Targets.Get(0)))
	{
		for (TWeakObjectPtr<AActor> Actor : ActorData->GetActors())
		{
			if (Actor.IsValid())
			{
				FVector Location = Actor->GetActorLocation();
				UUK_StatusAnimData* StatusAnimData = Player->GetNowWeaponStatus();
				if (IsValid(StatusAnimData) == false)
					return;
				UUK_AnimData* AnimData = StatusAnimData->FindAnimsDataAssetByType(AttackType);
				if (IsValid(AnimData) == false)
					return;
				if (IsValid(AnimData->HitSound) == false)
					return;
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					AnimData->HitSound,
					Location
				);
			}
		}
	}
}
