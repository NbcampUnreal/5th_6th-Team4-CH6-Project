// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GA/UK_LightAttackAbility.h"
#include "Character/UK_CharacterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DataAsset/UK_AnimData.h"
#include "Tags/UK_GameplayTags.h"

UUK_LightAttackAbility::UUK_LightAttackAbility() : index(1)
{
}

void UUK_LightAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}


UAnimMontage* UUK_LightAttackAbility::StartCombo(EComboAttackType InStatus)
{
	index = 1;
	AUK_CharacterBase* UKPC = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* PlayerMovement = UKPC->GetCharacterMovement();
	PlayerMovement->SetMovementMode(EMovementMode::MOVE_None);
	if ( IsValid(UKPC) == false )
	{
		EndCombo();
		return nullptr;
	}

	UUK_StatusAnimData* Status = UKPC->GetNowWeaponStatus();
	if ( IsValid(Status) == false )
	{
		EndCombo();
		return nullptr;
	}

	UUK_AnimData* AnimData = Status->FindAnimsDataAssetByType(InStatus);
	if ( IsValid(AnimData) == false )
	{
		EndCombo();
		return nullptr;
	}

	SectionString = AnimData->MontageSectionName;
	FName SectionName = *FString::Printf(TEXT("%s%d"), *AnimData->MontageSectionName, index);

	if ( IsValid(AnimData->ComboMantage) == false )
	{
		EndCombo();
		return nullptr;
	}
	return AnimData->ComboMantage;

}
void UUK_LightAttackAbility::ResetInput()
{
	AUK_CharacterBase* UKPC = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	if ( IsValid(UKPC) == false )
	{
		return;
	}
	UKPC->bIsInInput = false;
}
void UUK_LightAttackAbility::CharactorStopJump()
{
	AUK_CharacterBase* UKPC = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	UKPC->StopJumpAndFly();
}
void UUK_LightAttackAbility::ResetEndCombo()
{
	AUK_CharacterBase* UKPC = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
	UKPC->EndComboAttack();
}

void UUK_LightAttackAbility::EndCombo_Implementation()
{

}
//void UUK_LightAttackAbility::CurrentCombo()
//{
//	AUK_CharacterBase* UKPC = Cast<AUK_CharacterBase>(GetAvatarActorFromActorInfo());
//
//	UUK_StatusAnimData* Status = UKPC->GetNowWeaponStatus();
//
//	UKPC->bIsInInput = false;
//	++index;
//	//FName SectionName = *FString::Printf(TEXT("%s%d"), *AnimData->MontageSectionName, index);
//
//}

