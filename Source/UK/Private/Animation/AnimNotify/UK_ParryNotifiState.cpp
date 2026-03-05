// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_ParryNotifiState.h"
#include "Character/UK_CharacterBase.h"
#include "Tags/UK_GameplayTags.h"

void UUK_ParryNotifiState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (IAbilitySystemInterface* ASCActor = Cast<IAbilitySystemInterface>(MeshComp->GetOwner()))
	{
		ASCActor->GetAbilitySystemComponent()->AddLooseGameplayTag(UK_GameplayTags::Action::Parrying);
	}
}

void UUK_ParryNotifiState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (IAbilitySystemInterface* ASCActor = Cast<IAbilitySystemInterface>(MeshComp->GetOwner()))
	{
		ASCActor->GetAbilitySystemComponent()->RemoveLooseGameplayTag(UK_GameplayTags::Action::Parrying);
	}
}
