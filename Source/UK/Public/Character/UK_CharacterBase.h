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
class AUK_WeaponBase;
class UUK_InputConfig;
class UUK_InputComponent;
class UUK_CombatAnimationComponent;
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_CombatAnimationComponent> AnimationComponent;

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
	void Attack();

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void ZoomOut();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InputData")
	UUK_InputConfig* InputConfig;

	UPROPERTY()
	bool bSprint;
#pragma endregion

#pragma region Weapon
protected:
	UFUNCTION()
	virtual void OnRep_CurrentWeapon(const AUK_WeaponBase* OldWeapon);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<AUK_WeaponBase>> DefaultWeapons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "Weapon")
	TArray<TObjectPtr<AUK_WeaponBase>> Weapons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Category = "Weapon")
	TObjectPtr<AUK_WeaponBase> Weapon;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	int32 WeaponIndex;
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

	void DrawSweepCapsuleDebug(
		const FVector& Start,
		const FVector& End,
		float HalfHeight,
		const FColor& Color
		);
#pragma endregion
};
