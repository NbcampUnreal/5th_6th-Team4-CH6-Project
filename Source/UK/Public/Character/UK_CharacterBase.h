// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ActorComponent/StatusComponent.h"
#include "UK_CharacterBase.generated.h"



#pragma region Forward Declaration
class USpringArmComponent;
class UCameraComponent;
class UStatusComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
class AUK_WeaponBase;
class UUK_WeaponData;
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


	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
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
	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void ZoomIn();

	UFUNCTION(BlueprintCallable)
	void ZoomOut();

protected:

#pragma endregion

#pragma region Weapon
public:
	void EquipWeapon(AUK_WeaponBase* NewWeapon);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UUK_WeaponData> WeaponList;

	UPROPERTY()
	TObjectPtr<AUK_WeaponBase> CurrentWeapon;

#pragma endregion

#pragma region Battle
public:
	void ReceiveDamage(float Damage);
	float ApplyDamage();

	UFUNCTION()
	void Dead();

	UPROPERTY(BlueprintAssignable)
	FOnDeadDelegate OnDead;
#pragma endregion
};
