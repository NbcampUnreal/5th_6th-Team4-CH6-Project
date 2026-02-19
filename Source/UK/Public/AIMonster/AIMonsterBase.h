#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/AI_MonsterStatComponent.h"
#include "AIMonsterBase.generated.h"

class UBehaviorTree;
class UAnimMontage;

/* AI 상태 */
UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle,
	Patrol,
	Chase,
	Attack,
	Dead,
	Passive,
	Alert,
	Aggressive
};

/* 몬스터 성격 타입 */
UENUM(BlueprintType)
enum class EMonsterPersonality : uint8
{
	Aggressive,
	Peaceful
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, class AAIMonsterBase*, DeadMonster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterAttacked, class AAIMonsterBase*, AttackedMonster, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterStateChanged, EMonsterState, OldState, EMonsterState, NewState);

DECLARE_DELEGATE_OneParam(FOnAttackFinished, bool /*bSucceeded*/);

UCLASS(Abstract)
class UK_API AAIMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAIMonsterBase();

	virtual void PostInitializeComponents() override;

	UFUNCTION(Server, Reliable)
	void RequestState(EMonsterState NewState);

	UFUNCTION(BlueprintPure)
	EMonsterState GetCurrentState() const { return CurrentState; }

	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnMonsterStateChanged OnStateChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Personality")
	EMonsterPersonality Personality = EMonsterPersonality::Aggressive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Peaceful", meta = (EditCondition = "Personality == EMonsterPersonality::Peaceful"))
	float AlertDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Peaceful", meta = (EditCondition = "Personality == EMonsterPersonality::Peaceful"))
	float AllyCallRadius = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Peaceful", meta = (EditCondition = "Personality == EMonsterPersonality::Peaceful"))
	float ResetDistance = 2000.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsAggressive, BlueprintReadOnly, Category = "AI|Peaceful")
	bool bIsAggressive = false;

	UFUNCTION()
	void OnRep_IsAggressive();

	UFUNCTION(BlueprintPure, Category = "AI|Peaceful")
	bool GetIsAggressive() const { return bIsAggressive; }

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI|Peaceful")
	AActor* Aggressor = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AI|Peaceful")
	void CallNearbyAllies(AActor* Enemy);

	UFUNCTION(BlueprintCallable, Category = "AI|Peaceful")
	void ResetToPassive();

#pragma region Spawner System
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY()
	class AUK_MonsterSpawner* OwningSpawner;

	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	FVector SpawnLocation;

	UPROPERTY(BlueprintAssignable, Category = "Monster")
	FOnMonsterDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Monster")
	FOnMonsterAttacked OnAttacked;

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void Die();

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsDead() const { return CurrentState == EMonsterState::Dead || bIsDying; }

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Monster")
	UAI_MonsterStatComponent* GetStatComponent() const { return StatComponent; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LookAtRotationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxChaseDistance = 1500.0f;
#pragma endregion

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAI_MonsterStatComponent* StatComponent;

	UPROPERTY(ReplicatedUsing = OnRep_MonsterState)
	EMonsterState CurrentState = EMonsterState::Idle;

	UFUNCTION()
	void OnRep_MonsterState();

	void SetServerState(EMonsterState NewState);

public:
	void SetAIActive(bool bActive);

	virtual void OnIdle();
	virtual void OnChase(float DeltaSeconds);
	virtual void OnPatrol();
	virtual void OnAttack();
	virtual void OnDead();
	virtual void OnPassive();
	virtual void OnAlert();

#pragma region Combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	UAnimMontage* DeathMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	float DeathWithoutMontageDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	float CorpseLingerTime = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsDying = false;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool PlayRandomAttackMontage();

	FOnAttackFinished OnAttackFinished;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void FinalizeDeath();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(int32 MontageIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideCorpse();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetAppearance();

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 250.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 1.5f;

	float LastAttackTime = 0.f;
#pragma endregion

public:
	void ReceiveDamage(float Damage);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FTimerHandle CorpseTimerHandle;
	FTimerHandle DeathMontageTimerHandle;
	void HideAndBroadcastDeath();
};