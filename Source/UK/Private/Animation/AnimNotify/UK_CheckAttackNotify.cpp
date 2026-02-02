// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_CheckAttackNotify.h"
#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "Character/UK_CharacterBase.h"

void UUK_CheckAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

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
			CombatAnimComp->CheckComboProcessable(CombatAnimComp->GetNextAttackType());
		}

	}
}
