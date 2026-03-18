// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "UK_PlayerController.h"
#include "AIMonster/AIMonsterBase.h"
#include "DataAsset/HitMontageDataAsset.h"
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
class USoundAttenuation;
struct FInputActionValue;
struct FUK_WeaponItemData;
#pragma endregion

UENUM(BlueprintType)
enum class EInputMode : uint8
{
	None,
	Light,
	Heavy,
	Air,
	NormalSkill,
	UltimateSkill,
	Parry,
	Dash
};

// UENUM(BlueprintType)
// enum class ECustomMovementMode : uint8
// {
// 	CMOVE_None UMETA(DisplayName="None"),
// 	CMOVE_Glide UMETA(DisplayName="Glide"),
// 	CMOVE_Climb UMETA(DisplayName="Climb"),
// 	CMOVE_Swim UMETA(DisplayName="Swim")
// };

UENUM(BlueprintType)
enum class ECharacterAttribute : uint8
{
	None,
	Fire,
	Wind
};

DECLARE_DYNAMIC_DELEGATE(FOnFloorDelagate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeadDelagate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterAttribute, ECharacterAttribute, CharacterAttribute);

UCLASS()
class UK_API AUK_CharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

#pragma region Defualt
	// 함수
public:
	// Sets default values for this character's properties
	AUK_CharacterBase(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION(BlueprintCallable)
	void ChangedAttribute(ECharacterAttribute NewAttribute);
	void UpdateMovementState();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float GetFloorDistance();
	//geter, seter
public:
	TObjectPtr<USkeletalMeshComponent> GetRightHandWeapon() { return RightHandWeaponComponent; }
	TObjectPtr<USkeletalMeshComponent> GetLeftHandWeapon() { return LeftHandWeaponComponent; }
	TObjectPtr<UUK_InventoryComponent> GetInventoryComponent() { return InventoryComponent; }

	UFUNCTION(BlueprintCallable)
	ECharacterAttribute GetAttribute() const { return Attribute; }

	// 변수
protected:
#pragma region component

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_InventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;
#pragma endregion

public:
	UPROPERTY(editAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundAttenuation> Attenuation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GlideFallSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefualtGravity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefualtAirControl;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	ECharacterAttribute Attribute;

	// UPROPERTY(BlueprintReadWrite, EditAnywhere)
	// ECustomMovementMode MovementMode;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterAttribute OnChangedAttribute;

	UPROPERTY(EditAnywhere)
	UHitMontageDataAsset* HitMontageDataAsset;
	
	UPROPERTY(BlueprintReadWrite)
	bool bInWater = false;	
	UPROPERTY(BlueprintReadWrite)
	bool bWallDetected = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsGliding = false;
	
#pragma endregion

#pragma region Interaction And Quest

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_InteractionComponent> InteractionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_QuestComponent> QuestComp;

#pragma endregion

#pragma region GAS

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void GiveStartupAbilities();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GB|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
#pragma endregion

#pragma region Input
#pragma region MovementFunction

protected:
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
	void NomalSkill();

	UFUNCTION()
	void UltimateSkill();

	UFUNCTION()
	void Parry();

	UFUNCTION()
	void CrouchInput();

	UFUNCTION()
	void ToggleMouse();

	UFUNCTION()
	void Interaction();

	UFUNCTION()
	void Setting();

	UFUNCTION()
	void Dash();

#pragma endregion

public:
#pragma region LockOn

	UFUNCTION(BlueprintCallable)
	void LockON();

	UFUNCTION(BlueprintCallable)
	void LockONToggle();

	UFUNCTION(BlueprintCallable)
	void LockONTick();

	void AddTarget(const TObjectPtr<AAIMonsterBase> Monster);

	bool Locking() const { return bIsLock; }

	TArray<TObjectPtr<AAIMonsterBase>>& GetHitList() { return HitList; }

	void ResetHitList() { HitList.Reset(); }

protected:
	bool bIsLock;

	UPROPERTY()
	TArray<TObjectPtr<AAIMonsterBase>> LockOnList;

	UPROPERTY()
	TArray<TObjectPtr<AAIMonsterBase>> HitList;

	int32 index;

	FTimerHandle LockOnTimer;

	float MaxLockDistance = 1000.f;
#pragma endregion

#pragma region Gliding

	UFUNCTION(BlueprintCallable)
	bool StartGliding();

	UFUNCTION(BlueprintCallable)
	void EndGliding();
#pragma endregion

#pragma region Climb

public:
	UFUNCTION()
	bool ActorTrace();
	
	UFUNCTION()
	void Climb(FHitResult& Hit);
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float TraceDist;
	
	UPROPERTY()
	FHitResult HitResult;
	
	bool bIsClimb = false;
	
#pragma endregion

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UUK_InputConfig* InputMappingConfig;

	UPROPERTY()
	AUK_PlayerController* PC;

	UPROPERTY(BlueprintReadWrite)
	EInputMode InputType;

protected:
	bool bIsCrouched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSprinted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeed;


#pragma endregion

#pragma region Weapon

public:
	void ChangeWeaponStat(const FUK_WeaponItemData* WeaponStat);

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

	UUK_StatusAnimData* GetNowWeaponStatus() const { return NowWeapon; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> WeaponStatEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UUK_WeaponData> WeaponList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUK_StatusAnimData* NowWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 WeaponSlotIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FActiveGameplayEffectHandle WeaponEffectHandle;
#pragma endregion

#pragma region Battle

public:
	UFUNCTION(BlueprintCallable)
	void StopJumpAndFly();

	UFUNCTION(BlueprintCallable)
	void EndComboAttack();

	UFUNCTION()
	void Dead();

	UPROPERTY(BlueprintReadWrite)
	bool bIsFry;

	FOnFloorDelagate OnFloor;
	FOnDeadDelagate OnDead;

	UFUNCTION()
	void SetParry(const bool CheckParry) { bIsParry = CheckParry; }

	UFUNCTION()
	bool GetParry(const bool CheckParry) const { return CheckParry; }

	// void StartBattle();
	// void EndBattle();
protected:
	UPROPERTY()
	bool bIsParry;
	
	// bool bInBattle = false;
	//
	// FTimerHandle EndBattleTimerHandle;

#pragma endregion

#pragma region FindMonsterHPBar

public:
	void UpdateMonsterDetection();

	UPROPERTY()
	TSet<AAIMonsterBase*> NearbyMonsters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI/DetactBoundary")
	float DetectRadius = 1500.0f;

	FTimerHandle DetectTimer;

	bool bDrawDetectRadius = false;
#pragma endregion
};
