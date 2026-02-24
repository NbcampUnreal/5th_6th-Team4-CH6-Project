// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_DropAttackAnimNotify.h"
#include "Character/UK_CharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

void UUK_DropAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AUK_CharacterBase* OwnerCharacter = Cast<AUK_CharacterBase>(MeshComp->GetOwner());
	if ( !IsValid(OwnerCharacter) )
		return;
	if ( OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy )
		return;
	if ( OwnerCharacter->bIsInInput == false )
		return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);

	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag("Action.DropAttack");

	ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
}