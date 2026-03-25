// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None UMETA(DisplayName="None"),
	CMOVE_Climb UMETA(DisplayName="Climb"),
	CMOVE_Gliding UMETA(DisplayName="Gliding")
};

UCLASS()
class UK_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

protected:
	void PhysClimb(float deltaTime, int32 Iterations);
#pragma region Gliding
	void PhysGlide(float deltaTime, int32 Iterations);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Glide")
	float GlideFallSpeed;
#pragma endregion
	
	virtual bool IsWalkable(const FHitResult& Hit) const override;
	
};
