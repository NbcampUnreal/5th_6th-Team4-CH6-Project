// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GA/HitCheckAbility.h"
#include "AIMonster/AIMonsterBase.h"
#include "Character/UK_CharacterBase.h"
#include "Abilities/GameplayAbility.h"

void UHitCheckAbility::Attack(FGameplayAbilityTargetDataHandle TargetDataHandle)
{
	/*if (TargetDataHandle.Num() == 0)
		return;
	const FGameplayAbilityTargetData* BaseData = TargetDataHandle.Get(0);

	const FGameplayAbilityTargetData_ActorArray* ActorData =
		static_cast<const FGameplayAbilityTargetData_ActorArray*>(BaseData);

	if (!ActorData)
		return;

	TArray<TWeakObjectPtr<AActor>> Actors = ActorData->GetActors();

	if (Actors.Num() > 0)
	{
		for (const TWeakObjectPtr<AActor> Hit : Actors)
		{
			if (UKPC->HasAuthority())
			{
				if (TObjectPtr<AAIMonsterBase> Monster = Cast<AAIMonsterBase>(Hit))
				{
					bool bAlreadyHit = UKPC->GetHitList().Contains(Monster);
					if (bAlreadyHit == true)
						continue;
					UKPC->GetHitList().AddUnique(Monster);
					UKPC->AddTarget(Monster);
					Monster->ReceiveDamage(UKPC->ApplyDamage());
					//GetWorld()->UE_LOG(LogTemp, Warning, TEXT("Damage Applied to Monster: %s to Damage : %f"), *Monster->GetName(), OwnerCharactor->ApplyDamage());
				}
			}
		}
	}*/
	AUK_CharacterBase* UKPC = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	UKPC->LockON();
}
