// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "UK_CharacterBase.generated.h"

#define ECC_ATTACK ECollisionChannel::ECC_GameTraceChannel2

#pragma region Forward Declaration
class USpringArmComponent;
class UCameraComponent;
class UStatusComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
struct FInputActionValue;
#pragma endregion

UCLASS()
class UK_API AUK_CharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

#pragma region Defualt
public:
	// Sets default values for this character's properties
	AUK_CharacterBase();

	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStatusComponent> StatusComponent;

#pragma endregion
#pragma region GAS
protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void GiveStartupAbilities();

private:
	UPROPERTY(EditDefaultsOnly, Category = "GB|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
#pragma endregion

#pragma region Input
protected:
	UFUNCTION()
	void Sprint();

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void Attack();

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void ZoomOut();

protected:
	UPROPERTY()
	bool bSprint;
#pragma endregion
#pragma region Attack
public:
	virtual void BeginAttack();

	UFUNCTION()
	virtual void EndAttack(UAnimMontage* InMontage, bool bInterruped);

	UFUNCTION()
	void HandleOnCheckHit();

	UFUNCTION()
	void HandleOnCheckInputAttack();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	FString MontageSectionName = FString(TEXT("Attack"));

	int32 MaxComboCount = 3;

	int32 CurrentComboCount = 0;

	bool bIsNowAttacking = false;

	bool bIsAttackKeyPressed = false;

	FOnMontageEnded OnMeleeAttackMontageEndedDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRadius;

public:
	static int32 ShowAttackDebug;

#pragma endregion
};
