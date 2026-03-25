// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "UK_PlayerController.h"
#include "AIMonster/AIMonsterBase.h"
#include "DataAsset/HitMontageDataAsset.h"
#include "Systems/Data/UK_SaveInterface.h"
#include "UK_CharacterBase.generated.h"

#define ECC_LockOn ECollisionChannel::ECC_GameTraceChannel2
DECLARE_STATS_GROUP(TEXT("UK_Character"), STATGROUP_UK_Character, STATCAT_Advanced);

#pragma region Forward Declaration
class AUK_SoundManager;
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
class UNiagaraSystem;
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

UENUM(BlueprintType)
enum class ECharacterAttribute : uint8
{
	None,
	Fire,
	Wind
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeadDelagate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOutOfStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNormalSkill, float, CoolDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUltimateSkill, float, CoolDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterAttribute, ECharacterAttribute, CharacterAttribute);

UCLASS()
class UK_API AUK_CharacterBase : public ACharacter, public IAbilitySystemInterface, public IUK_SaveInterface
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
#pragma region component
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UUK_InventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;
#pragma endregion
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundAttenuation> Attenuation;

	UPROPERTY()
	AUK_SoundManager* SoundManager;
	
	UPROPERTY(BlueprintReadOnly)
	float GlideFallSpeed;

	UPROPERTY(BlueprintReadOnly)
	float DefualtGravity;

	UPROPERTY(BlueprintReadOnly)
	float DefualtAirControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed;
	
	UPROPERTY(BlueprintReadWrite)
	ECharacterAttribute Attribute;

	// UPROPERTY(BlueprintReadWrite, EditAnywhere)
	// ECustomMovementMode MovementMode;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UHitMontageDataAsset* HitMontageDataAsset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UUK_InputConfig* InputMappingConfig;
	
	UPROPERTY()
	AUK_PlayerController* PC;

#pragma region Status
public:
	void HealStamina();
	
	void OutOfStamina();
	
public:
	UPROPERTY(BlueprintReadWrite)
	EInputMode InputType;
	
protected:
	
	UPROPERTY()
	bool bIsParry;
	
	bool bInBattle = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsSprinted = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsFry;

	UPROPERTY(BlueprintReadWrite)
	bool bInWater = false;	
	
	UPROPERTY(BlueprintReadWrite)
	bool bWallDetected = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsGliding = false;
	
	bool bIsClimb = false;
	
	bool bInUseStamina = false;
#pragma endregion
	
#pragma region Delegate
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "CoolDown", meta = ( DisplayNmae = "OnNomalSkillCoolDown" ))
	FOnNormalSkill OnNormalSkillCoolDownDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "CoolDown", meta = ( DisplayNmae = "OnNomalSkillCoolDown" ))
	FOnUltimateSkill OnUltimateSkillCoolDownDelegate;

	FOnDeadDelagate OnDead;
	
	UPROPERTY(BlueprintAssignable)
	FOutOfStamina OutOfStaminaHandle;
	
	UPROPERTY(BlueprintAssignable)
	FOnCharacterAttribute OnChangedAttribute;
#pragma endregion
	
#pragma region TimerHandle
	FTimerHandle GlidingTimer;
	
	FTimerHandle DetectTimer;
	
	FTimerHandle LockOnTimer;
	
	FTimerHandle StaminaHealTimerHandle;
	
	FTimerHandle EndBattleTimerHandle;
#pragma endregion
	
#pragma endregion
	
#pragma  region SaveGame
public:
	virtual void OnLoadGame(class UUK_InGameSave* SaveGameObject) override;
	virtual void OnSaveGame(class UUK_InGameSave* SaveGameObject) override;

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
#pragma region GameplayEffect
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> HealStaminaEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> WeaponStatEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EndBattleEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> ResurrectionEffect;
	
	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle HealStaminaEffectHandle;	
	
	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle WeaponEffectHandle;
	
	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle EndBattleEffectHandle;
	
	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle ResurrectionEffectHandle;
#pragma endregion
	
#pragma endregion

#pragma region Input
	
#pragma region MovementFunction
protected:
	UFUNCTION()
	void Move(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void Look(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void ZoomOut();

	UFUNCTION()
	void LightAttack();

	UFUNCTION()
	void HeavyAttack();

	UFUNCTION()
	void NormalSkill();

	UFUNCTION()
	void UltimateSkill();

	UFUNCTION()
	void Parry();

	UFUNCTION()
	void ToggleMouse();

	UFUNCTION()
	void Interaction();

	UFUNCTION()
	void Setting();

	UFUNCTION()
	void Inventory();

	UFUNCTION()
	void WeaponCrafting();

	UFUNCTION()
	void Esc();

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

	int32 LockOnIndex;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
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
	
#pragma endregion

public:
	UFUNCTION(BlueprintCallable)
	void StartSprintCost();
	UFUNCTION(BlueprintCallable)
	void EndSprintCost();

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

	UFUNCTION(BlueprintCallable)
	UUK_StatusAnimData* GetNowWeaponStatus() const { return NowWeapon; }

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UUK_WeaponData> WeaponList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(BlueprintReadOnly)
	UUK_StatusAnimData* NowWeapon;
	
	UPROPERTY(BlueprintReadOnly)
	int32 WeaponSlotIndex;

#pragma endregion

#pragma region Battle

public:
	UFUNCTION(BlueprintCallable)
	void StopJumpAndFly();

	UFUNCTION(BlueprintCallable)
	void EndComboAttack();

	UFUNCTION()
	void Dead();	
	UFUNCTION(BlueprintCallable)
	void Resurrection();

	UFUNCTION()
	void SetParry(const bool CheckParry) { bIsParry = CheckParry; }

	UFUNCTION()
	bool GetParry(const bool CheckParry) const { return CheckParry; }

	void StartBattle();
	
	void EndBattle();

	// 레벨업 로직 함수
	void HandleLevelUp();
protected:
	// 레벨업 시 재생할 나이아가라 이펙트
	UPROPERTY(EditAnywhere, Category = "UK|LevelUp")
	TObjectPtr<UNiagaraSystem> LevelUpVFX;

	// 레벨업 시 재생할 사운드
	UPROPERTY(EditAnywhere, Category = "UK|LevelUp")
	TObjectPtr<USoundBase> LevelUpSound;


#pragma endregion

#pragma region FindMonsterHPBar

public:
	void UpdateMonsterDetection();

	UPROPERTY()
	TSet<AAIMonsterBase*> NearbyMonsters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI/DetactBoundary")
	float DetectRadius = 1500.0f;

	bool bDrawDetectRadius = false;
#pragma endregion

};
