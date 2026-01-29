// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UK_AnimInstance.generated.h"

#pragma region Forward Declaration
class UCharacterMovementComponent;
class AUK_CharacterBase;
#pragma endregion

#pragma region DELEGATE
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckHit);
#pragma endregion

UCLASS()
class UK_API UUK_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
#pragma region Defualt

public:

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AUK_CharacterBase> OwnerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovement;
#pragma endregion

#pragma region MovementValue
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GroundSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bShouldMove : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bIsFalling : 1;
#pragma endregion
#pragma region DELEGATE
private:
	UFUNCTION()
	void AnimNotifyCheckHit();

public:
	FOnCheckHit OnCheckHit;
#pragma endregion
};
