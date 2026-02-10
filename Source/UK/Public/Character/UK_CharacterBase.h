// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ActorComponent/StatusComponent.h"
#include "GameplayTagContainer.h"
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
class UUK_InventoryComponent;
class UAIPerceptionStimuliSourceComponent;
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
	TObjectPtr<USkeletalMeshComponent> GetRightHandWeapon() { return RightHandWeaponComponent; }
	TObjectPtr<USkeletalMeshComponent> GetLeftHandWeapon() { return LeftHandWeaponComponent; }
	TObjectPtr<UUK_InventoryComponent> GetInventoryComponent() { return InventoryComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MannySkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
	TObjectPtr<USkeletalMeshComponent> RightHandWeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
	TObjectPtr<USkeletalMeshComponent> LeftHandWeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_CombatAnimationComponent> AnimationComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_InventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

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
	void EquipWeapon(FGameplayTag NewWeapon);

	UFUNCTION(BlueprintCallable)
	void SlotWeaponOne();
	UFUNCTION(BlueprintCallable)
	void SlotWeaponTwo();
	UFUNCTION(BlueprintCallable)
	void SlotWeaponThree();

	void SwapWeapon(int32 Index);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UUK_WeaponData> WeaponList;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UDataTable> ItmeDataTable;

	FGameplayTag NowWeapon;
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
