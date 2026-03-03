#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "UK_EliteMonster.generated.h"

/**
 * 엘리트 몬스터 베이스
 * ■ 특수 공격 : SpecialAttackMontages 재생.
 *              데미지는 AnimNotifyState_UKMonsterMeleeTrace 가 처리.
 */
UCLASS(Abstract)
class UK_API AUK_EliteMonster : public AAIMonsterBase
{
	GENERATED_BODY()

#pragma region Initialization
public:
	AUK_EliteMonster();
#pragma endregion

#pragma region Special Attack
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|SpecialAttack")
	TArray<UAnimMontage*> SpecialAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|SpecialAttack")
	float SpecialAttackCooldown = 8.0f;

	float LastSpecialAttackTime = 0.f;

	UFUNCTION(BlueprintPure, Category = "Elite|SpecialAttack")
	bool CanUseSpecialAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Elite|SpecialAttack")
	bool PlaySpecialAttack();

	DECLARE_DELEGATE_OneParam(FOnSpecialAttackFinished, bool /*bSucceeded*/);
	FOnSpecialAttackFinished OnSpecialAttackFinished;

	UFUNCTION()
	void OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpecialAttackMontage(int32 MontageIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Attack")
	float SpecialAttackAoERadius = 500.f;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Attack")
	float SpecialAttackDamage = 50.f;     
	
	FTimerHandle SpecialAttackAoETimerHandle;

	UPROPERTY(EditAnywhere, Category = "Special Attack")
	float SpecialAttackHitTiming = 0.4f;

	void ApplySpecialAttackAoE();
#pragma endregion

#pragma region Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug")
	bool bShowSpecialAttackDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugRadius = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugTraceHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugTraceLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugTraceRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugDuration = 1.5f;
#pragma endregion

#pragma region Replication
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#pragma endregion
};