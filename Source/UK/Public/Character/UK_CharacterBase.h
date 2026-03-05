// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ActorComponent/StatusComponent.h"
#include "GameplayTagContainer.h"
#include "UK_PlayerController.h"
#include "AIMonster/AIMonsterBase.h"
#include "UK_CharacterBase.generated.h"

#define ECC_LockOn ECollisionChannel::ECC_GameTraceChannel2



#pragma region Forward Declaration
class USpringArmComponent;
class UCameraComponent;
class UStatusComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
class AUK_WeaponBase;
class UUK_WeaponData;
class UUK_StatusAnimData;
class UUK_CombatAnimationComponent;
class UUK_InventoryComponent;
class UAIPerceptionStimuliSourceComponent;
class AAIMonsterBase;
class UUK_InputConfig;
class UUK_InteractionComponent;
class UUK_QuestComponent;
struct FInputActionValue;
#pragma endregion

DECLARE_DYNAMIC_DELEGATE(FOnFloorDelagate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeadDelagate);
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

	virtual void Landed(const FHitResult& Hit) override;

	virtual void OnRep_PlayerState();
	TObjectPtr<USkeletalMeshComponent> GetRightHandWeapon() { return RightHandWeaponComponent; }
	TObjectPtr<USkeletalMeshComponent> GetLeftHandWeapon() { return LeftHandWeaponComponent; }
	TObjectPtr<UUK_InventoryComponent> GetInventoryComponent() { return InventoryComponent; }
	void OnRep_RightHandWeapon();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;
	 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> RightHandWeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> LeftHandWeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_InventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;
#pragma endregion

#pragma region Interaction And Quest
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_InteractionComponent> InteractionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_QuestComponent> QuestComp;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UUK_InputConfig* InputMappingConfig;
protected:
	UPROPERTY()
	AUK_PlayerController* PC;

	UFUNCTION()
	void Move(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void Look(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void Sprint();

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void ZoomOut();
	
	UFUNCTION()
	void LightAttack();

	UFUNCTION()
	void HeavyAttack();

	UFUNCTION()
	void CrouchInput();

	UFUNCTION()
	void ToggleMouse();

	UFUNCTION()
	void Interaction();

	UFUNCTION()
	void Setting();

	UFUNCTION()
	void NomalSkill();

	UFUNCTION()
	void UltimateSkill();

public:
	UFUNCTION(BlueprintCallable)
	void LockON();

	UFUNCTION(BlueprintCallable)
	void LockONToggle();

	UFUNCTION(BlueprintCallable)
	void LockONTick();

	void AddTarget(const TObjectPtr<AAIMonsterBase> Monster);

	bool Locking()const { return bIsLock; }

	TArray<TObjectPtr<AAIMonsterBase>>& GetHitList() { return HitList; }

	void ResetHitList() { HitList.Reset(); }
protected:

	bool bIsLock;

	bool bIsCrouched;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bIsSprinted;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float SprintSpeed;

	UPROPERTY()
	TArray<TObjectPtr<AAIMonsterBase>> LockOnList;

	UPROPERTY()
	TArray<TObjectPtr<AAIMonsterBase>> HitList;

	int32 index;

	FTimerHandle LockOnTimer;
	float MaxLockDistance = 1000.f;
#pragma endregion

#pragma region Weapon
public:
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(FGameplayTag NewWeapon);

	UFUNCTION(BlueprintCallable)
	void SlotWeaponOne();
	UFUNCTION(BlueprintCallable)
	void SlotWeaponTwo();
	UFUNCTION(BlueprintCallable)
	void SlotWeaponThree();

	UFUNCTION(BlueprintCallable)
	void SwapWeapon(int32 Index);


	UFUNCTION()
	void OnRep_CurrentWeaponTag();
	UFUNCTION()
	void OnRep_NowWeapon();

	UUK_StatusAnimData* GetNowWeaponStatus() const { return NowWeapon; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UUK_WeaponData> WeaponList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_NowWeapon)
	UUK_StatusAnimData* NowWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UDataTable> ItmeDataTable;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponTag)
	FGameplayTag CurrentWeaponTag;
#pragma endregion

#pragma region Battle
public:
	UFUNCTION(BlueprintCallable)
	void StopJumpAndFly();

	UFUNCTION(BlueprintCallable)
	void EndComboAttack();

	void ReceiveDamage(float Damage);

	float ApplyDamage();

	UFUNCTION()
	void Dead();

	UFUNCTION()
	void OnRep_InInput();

	UFUNCTION()
	void OnRep_fry();

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_InInput)
	bool bIsInInput = false;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_fry)
	bool bIsfry;

	FOnFloorDelagate OnFloor;
	FOnDeadDelagate OnDead;
#pragma endregion

#pragma region FindMonsterHPBar
	public:

		void UpdateMonsterDetection();

		UPROPERTY()
		TSet<AAIMonsterBase*> NearbyMonsters;

		UPROPERTY(BlueprintReadWrite, EditAnywhere,  Category = "UI/DetactBoundary")
		float DetectRadius = 1000.0f;

		FTimerHandle DetectTimer;
#pragma endregion
};
