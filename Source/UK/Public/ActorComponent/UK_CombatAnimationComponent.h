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
#pragma region Defualt

public:	
	// Sets default values for this component's properties
	UUK_CombatAnimationComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps)const override;
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentComboCount, BlueprintReadOnly, Category = "Combat")
	uint8 CurrentComboCount;

	float DefaultGravityValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AUK_CharacterBase> OwnerCharactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUK_AnimData> AttackAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUK_StatusAnimData> NowWeapon;

	EAttackInput InputType;
#pragma endregion

public:
	UFUNCTION()
	void OnRep_CurrentComboCount();

	void PlayLightComboAnimation();

#pragma region ServerRPCs
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCombo(EComboAttackType AttackType, uint8 ComboCount);

	UFUNCTION(Server, Reliable)
	void ServerRPCStartComboAttack(const EComboAttackType AttackType);

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttack(const EComboAttackType AttackType, FName SectionName);
#pragma endregion

	void PlayComboAttackAnimation(const EComboAttackType AttackType, FName SectionName);

	void StopJumpAndFly();
#pragma region EndCombo

	void EndComboAttack(UAnimMontage* TargetMontage, bool bInterrupted);
	void ResetCharacterGravityScale();
	void ResetPlayerComboAttackValue();
	void ResetPlayerCharacterMovement();
#pragma endregion

	void CheckComboProcessable(const EComboAttackType AttackType);

	EComboAttackType GetNextAttackType();

#pragma region Battle
public:
	static int32 ShowAttackDebug;

	FORCEINLINE void SetDamageEvent(FDamageEvent NewDamageEvent) {DamageEvent = NewDamageEvent	;}

	void SetEnableHitCheck(bool bEnablaHitCheck);

	void HitCheckProcess();

	void SetNowWeapon(const TObjectPtr<UUK_StatusAnimData>& Weapon);

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
#pragma endregion

};
