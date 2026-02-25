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

/* 몬스터 종류 (보상 지급용) */
UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	None = 0,
	EliteGolem = 1,
	Golem = 2,
	Wolf = 3,
	Fox = 4,
	Reindeer = 5,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, class AAIMonsterBase*, DeadMonster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterAttacked, class AAIMonsterBase*, AttackedMonster, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterStateChanged, EMonsterState, OldState, EMonsterState, NewState);

/* 몬스터 킬 알림 델리게이트 (서버 → GameMode) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMonsterKilled, 
	class AAIMonsterBase*, KilledMonster,
	EMonsterType, MonsterType,
	class APlayerController*, KillerController
);

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

	/* 몬스터 종류 (에디터에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	EMonsterType MonsterType = EMonsterType::None;

	/* 몬스터 킬 알림 (GameMode가 Subscribe) */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterKilled OnMonsterKilled;

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

	/* 마지막 공격자 추적 (보상 지급용) */
	UPROPERTY(Replicated)
	APlayerController* LastAttackerController = nullptr;

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
	void Multicast_HideCorpse();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetAppearance();

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 250.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 0.5f;

	float LastAttackTime = 0.f;
#pragma endregion

#pragma region Idle Animation
	/** 배회 중 목적지 도착 시 재생할 아이들 몽타주 목록 (평화 몬스터용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Animation",
		meta = (EditCondition = "Personality == EMonsterPersonality::Peaceful"))
	TArray<UAnimMontage*> IdleMontages;

	/**
	 * 랜덤 아이들 몽타주 재생 (Multicast)
	 * @return 재생 성공 여부 (IdleMontages가 비어있으면 false)
	 */
	UFUNCTION(BlueprintCallable, Category = "Idle|Animation")
	bool PlayRandomIdleMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayIdleMontage(int32 MontageIndex);

	/** 아이들 몽타주 재생 완료 콜백 */
	UFUNCTION()
	void OnIdleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 아이들 몽타주 종료 시 BT 태스크에 알림 */
	DECLARE_DELEGATE(FOnIdleMontageFinished);
	FOnIdleMontageFinished OnIdleMontageFinished;
#pragma endregion
	
#pragma region Hit Animation
	/** 피격 시 재생할 몽타주 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Animation")
	TArray<UAnimMontage*> HitMontages;

	/**
	 * 랜덤 피격 몽타주 재생 (Multicast)
	 * @return 재생 성공 여부 (HitMontages가 비어있으면 false)
	 */
	UFUNCTION(BlueprintCallable, Category = "Hit|Animation")
	bool PlayRandomHitMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitMontage(int32 MontageIndex);
#pragma endregion
	
public:
	void ReceiveDamage(float Damage);

	/* 데미지를 준 공격자 추적 */
	void ReceiveDamageFrom(float Damage, AController* InstigatorController);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FTimerHandle CorpseTimerHandle;
	void HideAndBroadcastDeath();
	
	/* 킬 알림 전송 */
	void NotifyMonsterKilled();
};