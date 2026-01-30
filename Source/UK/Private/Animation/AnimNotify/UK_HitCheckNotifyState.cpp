// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_HitCheckNotifyState.h"
#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h"

void UUK_HitCheckNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if ( !IsValid( OwnerCharacter) )
		return;
	if ( OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy )
		return;

	const TSet<UActorComponent*>& CharacterComponents = OwnerCharacter->GetComponents();
	for ( UActorComponent* const Actorcomp : CharacterComponents )
	{
		UUK_CombatAnimationComponent* CombatAnimComp = Cast<UUK_CombatAnimationComponent>(Actorcomp);
		if ( IsValid(CombatAnimComp) )
		{
			FDamageEvent NewDamageEvent(AttackDamageType);
			CombatAnimComp->SetEnableHitCheck(true);
			CombatAnimComp->SetDamageEvent(NewDamageEvent);
		}

	}
}

void UUK_HitCheckNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if ( !IsValid(OwnerCharacter) )
		return;
	if ( OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy )
		return;

	const TSet<UActorComponent*>& CharacterComponents = OwnerCharacter->GetComponents();
	for ( UActorComponent* const Actorcomp : CharacterComponents )
	{
		UUK_CombatAnimationComponent* CombatAnimComp = Cast<UUK_CombatAnimationComponent>(Actorcomp);
		if ( IsValid(CombatAnimComp) )
		{
			CombatAnimComp->SetEnableHitCheck(false);
		}
	}
}
