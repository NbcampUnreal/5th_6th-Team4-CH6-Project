// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/UK_AnimInstance.h"
#include "Character/UK_CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UUK_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	TObjectPtr< APawn > OwnerPawn = TryGetPawnOwner();
	if (OwnerPawn) 
	{
		OwnerCharacter = Cast<AUK_CharacterBase>(OwnerPawn);
		OwnerCharacterMovement = OwnerCharacter->GetCharacterMovement();
	}
}

void UUK_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter && OwnerCharacterMovement)
	{
		Velocity = OwnerCharacterMovement->Velocity;
		GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);

		const float GroundAcceleration = UKismetMathLibrary::VSizeXY(OwnerCharacterMovement->GetCurrentAcceleration());
		const bool bIsAccelerationNearlyZero = FMath::IsNearlyZero(GroundAcceleration);

		bShouldMove = (KINDA_SMALL_NUMBER < GroundSpeed) && (!bIsAccelerationNearlyZero);
		bIsFalling = OwnerCharacterMovement->IsFalling();
	}
}

void UUK_AnimInstance::AnimNotifyCheckHit()
{
	if (OnCheckHit.IsBound())
	{
		OnCheckHit.Broadcast();
	}
}
