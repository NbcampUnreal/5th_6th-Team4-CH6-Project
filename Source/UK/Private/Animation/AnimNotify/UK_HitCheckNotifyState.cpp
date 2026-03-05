// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_HitCheckNotifyState.h"
#include "Character/UK_CharacterBase.h"
#include "Tags/UK_GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

void UUK_HitCheckNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	OwnerCharacter = Cast<AUK_CharacterBase>(MeshComp->GetOwner());
	if ( !IsValid(OwnerCharacter) )
		return;
	if ( OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy )
		return;
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(
		HitTimer,
		this,
		&ThisClass::HitCheck,
		0.05f,
		true
	);

}

void UUK_HitCheckNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if ( IsValid(OwnerCharacter) )
	{
		OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(HitTimer);
		OwnerCharacter->ResetHitList();
		OwnerCharacter = nullptr;
	}
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UUK_HitCheckNotifyState::HitCheck()
{

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerCharacter, UK_GameplayTags::Attack::HitCheck, EventData);
}
