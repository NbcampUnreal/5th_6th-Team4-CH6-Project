// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "UK_CombatAnimationComponent.generated.h"

#define ECC_ATTACK ECollisionChannel::ECC_GameTraceChannel2

#pragma region Forward Declaration
class AUK_CharacterBase;
class UUK_StatusAnimData;
class UUK_WeaponData;
class UUK_AnimData;
#pragma endregion

UENUM()
enum class EAttackInput : uint8
{
	None,
	Light,
	Heavy
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UUK_CombatAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUK_CombatAnimationComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps)const override;
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetNowWeapon(const TObjectPtr<UUK_StatusAnimData>& Weapon);
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	uint8 CurrentComboCount;
	float DefaultGravityValue;

	FTimerHandle ComboCheckTimer;


public:
	void PlayLightComboAnimation();

	void StartComboAttack(const EComboAttackType AttackType);

	UFUNCTION(Server, Reliable)
	void ServerRPCStartComboAttack(const EComboAttackType AttackType);

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttack(const EComboAttackType AttackType, FName SectionName);

	void PlayComboAttackAnimation(const EComboAttackType AttackType, FName SectionName);

	void SetCheckComboTimer(const EComboAttackType AttackType);

	void StopJumpAndFly();

	void EndComboAttack(UAnimMontage* TargetMontage, bool bInterrupted);

	void ResetCharacterGravityScale();
	void ResetPlayerComboAttackValue();
	void ResetPlayerCharacterMovement();

	void CheckComboProcessable(const EComboAttackType AttackType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCombo(EComboAttackType AttackType, uint8 ComboCount);

	UFUNCTION(Server, Reliable)
	void ServerResetPlayerComboAttackValue();

	EComboAttackType GetNextAttackType();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AUK_CharacterBase> OwnerCharactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUK_AnimData> AttackAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUK_StatusAnimData> NowWeapon;



	EAttackInput InputType;

	UPROPERTY(Replicated)
	uint8 ServerComboCount;

	UPROPERTY(Replicated)
	EComboAttackType ServerAttackType;

public:
	static int32 ShowAttackDebug;
	FORCEINLINE void SetDamageEvent(FDamageEvent NewDamageEvent) {DamageEvent = NewDamageEvent	;}

	void SetEnableHitCheck(bool bEnablaHitCheck);

	void HitCheckProcess();

	UFUNCTION(BlueprintCallable)
	void SetWeaponMesh(UStaticMeshComponent* NewWeapon);

protected:
	UPROPERTY()
	FDamageEvent DamageEvent;

	FTimerHandle HitCheckTimer;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FName TraceStartSocketName;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FName TraceEndSocketName;

	UPROPERTY()
	TSet<AActor*> HitcheckedActor;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
};
