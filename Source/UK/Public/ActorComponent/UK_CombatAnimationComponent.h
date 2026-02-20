// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "UK_CombatAnimationComponent.generated.h"

#define ECC_ATTACK ECollisionChannel::ECC_GameTraceChannel1

#pragma region Forward Declaration
class AUK_CharacterBase;
class UUK_StatusAnimData;
class UUK_WeaponData;
class UUK_AnimData;
class AUK_WeaponBase;
#pragma endregion

UENUM()
enum class EAttackInput : uint8
{
	None,
	Light,
	Heavy,
	NomalSkill,
	UltimateSkill
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UUK_CombatAnimationComponent : public UActorComponent
{
	GENERATED_BODY()
#pragma region Defualt

public:	
	// Sets default values for this component's properties
	UUK_CombatAnimationComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps)const override;
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// Called when the game starts
	virtual void BeginPlay() override;
	uint8 GetCurrentComboCount() const { return CurrentComboCount; }
protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentComboCount, BlueprintReadOnly, Category = "Combat")
	uint8 CurrentComboCount;

	float DefaultGravityValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AUK_CharacterBase> OwnerCharactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUK_AnimData> AttackAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	UUK_StatusAnimData* NowWeapon;

	EAttackInput InputType;
#pragma endregion

public:
	UFUNCTION()
	void OnRep_CurrentComboCount();

	void PlayLightComboAnimation();
	void PlayHeavyComboAnimation();
	void PlayNomalSkillComboAnimation();
	void PlayUltimateSkillComboAnimation();

#pragma region ServerRPCs
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCombo(EComboAttackType AttackType, uint8 ComboCount);

	UFUNCTION(Server, Reliable)
	void ServerRPCStartComboAttack(const EComboAttackType AttackType);

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttack(const EComboAttackType AttackType, FName SectionName);

	UFUNCTION(Server, Reliable)
	void ServerRPCDropAttack();

	UFUNCTION(Server, Reliable)
	void ServerRPCDropOnFloorAttack();

#pragma endregion

	void PlayComboAttackAnimation(const EComboAttackType AttackType, FName SectionName);

	void StopJumpAndFly();
#pragma region EndCombo

	UFUNCTION(BlueprintCallable)
	void EndComboAttack(UAnimMontage* TargetMontage, bool bInterrupted);
	void ResetCharacterGravityScale();
	void ResetPlayerComboAttackValue();
	void ResetPlayerCharacterMovement();
#pragma endregion

	void CheckComboProcessable(const EComboAttackType AttackType);
	void CheckDropAttackProcessable();

	EComboAttackType GetNextAttackType();

#pragma region Battle
public:
	static int32 ShowAttackDebug;

	FORCEINLINE void SetDamageEvent(FDamageEvent NewDamageEvent) {DamageEvent = NewDamageEvent	;}

	void SetEnableRightHitCheck(bool bEnablaHitCheck);

	void RightHitCheckProcess();

	void SetEnableLeftHitCheck(bool bEnablaHitCheck);

	void LeftHitCheckProcess();


protected:
	UPROPERTY()
	FDamageEvent DamageEvent;

	FTimerHandle RightHitCheckTimer;
	FTimerHandle LeftHitCheckTimer;

	UPROPERTY()
	TSet<AActor*> RightHitcheckedActor;
	UPROPERTY()
	TSet<AActor*> LeftHitcheckedActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SoundDistanece")
	TObjectPtr< USoundAttenuation > SoundAttenuation;

	UFUNCTION(Server, Reliable)
	void ServerRPCPlaySoundAndEffect(USoundBase* Sound);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySoundAndEffect(USoundBase* Sound);
#pragma endregion

#pragma region Weapon
public:
	void SetNowWeapon(UUK_StatusAnimData* NewWeapon);
	UFUNCTION(Server, Reliable)
	void ServerRPCSetNowWeapon(UUK_StatusAnimData* NewWeapon);
protected:

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FName TraceStartSocketName;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FName TraceEndSocketName;
#pragma endregion

};
