#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "UI/InGame/UK_MonsterHealthBar.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "AIMonsterBase.generated.h"

class UBehaviorTree;
class UAnimMontage;
class UAbilitySystemComponent;
class UUK_MonsterAttributeSet;
class UGameplayEffect;
class USoundCue;

/* ───────────────────── Enums & Delegates ───────────────────── */

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle, Patrol, Chase, Attack, Dead, Passive, Alert, Aggressive
};

UENUM(BlueprintType)
enum class EMonsterPersonality : uint8
{
	Aggressive, Peaceful
};

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	None       = 0,
	EliteGolem = 1,
	Golem      = 2,
	Wolf       = 3,
	Fox        = 4,
	Reindeer   = 5,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, class AAIMonsterBase*, DeadMonster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterAttacked, class AAIMonsterBase*, AttackedMonster, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterStateChanged, EMonsterState, OldState, EMonsterState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMonsterKilled,
	class AAIMonsterBase*, KilledMonster,
	EMonsterType, MonsterType,
	class APlayerController*, KillerController);

DECLARE_DELEGATE_OneParam(FOnAttackFinished, bool /*bSucceeded*/);

/* ─────────────────────────────────────────────────────────────── */

UCLASS(Abstract)
class UK_API AAIMonsterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

#pragma region Initialization
public:
	AAIMonsterBase();
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
#pragma endregion

#pragma region Ability System
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Abilities")
	UUK_MonsterAttributeSet* GetMonsterAttributeSet() const { return AttributeSet; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UUK_MonsterAttributeSet* AttributeSet;

	/** 데미지 적용용 GameplayEffect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
#pragma endregion

#pragma region State Management
public:
	UFUNCTION(BlueprintCallable, Category = "AI")
	void RequestState(EMonsterState NewState);

	UFUNCTION(BlueprintPure)
	EMonsterState GetCurrentState() const { return CurrentState; }

	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnMonsterStateChanged OnStateChanged;

	void SetAIActive(bool bActive);

	virtual void OnIdle();
	virtual void OnChase(float DeltaSeconds);
	virtual void OnPatrol();
	virtual void OnAttack();
	virtual void OnDead();
	virtual void OnPassive();
	virtual void OnAlert();

protected:
	EMonsterState CurrentState = EMonsterState::Idle;
	void SetState(EMonsterState NewState);
#pragma endregion

#pragma region Personality
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Personality")
	EMonsterPersonality Personality = EMonsterPersonality::Aggressive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Type")
	EMonsterType MonsterType = EMonsterType::None;
#pragma endregion

#pragma region Peaceful AI
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Peaceful")
	float AlertDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Peaceful")
	float AllyCallRadius = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Peaceful")
	float ResetDistance = 2000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Peaceful")
	bool bIsAggressive = false;

	UFUNCTION(BlueprintPure, Category = "AI|Peaceful")
	bool GetIsAggressive() const { return bIsAggressive; }

	UPROPERTY(BlueprintReadOnly, Category = "AI|Peaceful")
	AActor* Aggressor = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AI|Peaceful")
	void CallNearbyAllies(AActor* Enemy);

	UFUNCTION(BlueprintCallable, Category = "AI|Peaceful")
	void ResetToPassive();
#pragma endregion

#pragma region Spawner System
public:
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

	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterKilled OnMonsterKilled;

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void Die();

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsDead() const { return CurrentState == EMonsterState::Dead || bIsDying; }

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void ResetHealth();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LookAtRotationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxChaseDistance = 1500.0f;

protected:
	UPROPERTY()
	APlayerController* LastAttackerController = nullptr;
#pragma endregion

#pragma region Combat
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	float CorpseLingerTime = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsDying = false;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 250.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 0.5f;

	float LastAttackTime = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual bool PlayRandomAttackMontage();

	FOnAttackFinished OnAttackFinished;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void FinalizeDeath();
	void PlayAttackMontage(int32 MontageIndex);
	void HideCorpse();
	void ResetAppearance();

	/** GAS를 통한 데미지 적용 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamage(float DamageAmount, AController* InstigatorController = nullptr);

	/** 레거시 호환용 */
	virtual void ReceiveDamage(float Damage);
	void ReceiveDamageFrom(float Damage, AController* InstigatorController);
#pragma endregion

#pragma region Idle Animation
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Animation")
	TArray<UAnimMontage*> IdleMontages;

	UFUNCTION(BlueprintCallable, Category = "Idle|Animation")
	virtual bool PlayRandomIdleMontage();

	void PlayIdleMontage(int32 MontageIndex);

	UFUNCTION()
	void OnIdleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	DECLARE_DELEGATE(FOnIdleMontageFinished);
	FOnIdleMontageFinished OnIdleMontageFinished;
#pragma endregion

#pragma region Hit Animation
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Animation")
	TArray<UAnimMontage*> HitMontages;

	UPROPERTY(BlueprintReadOnly, Category = "Hit|Animation")
	bool bIsHit = false;

	UFUNCTION(BlueprintCallable, Category = "Hit|Animation")
	bool PlayRandomHitMontage();

	void PlayHitMontage(int32 MontageIndex);

	UFUNCTION()
	void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

#pragma region HP Bar Widget
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FTimerHandle CorpseTimerHandle;
	void HideAndBroadcastDeath();
	
	/* 킬 알림 전송 */
	void NotifyMonsterKilled();
#pragma region HPBar Widget
public:

	UUK_MonsterHealthBar* GetHPWidget() const;

	virtual void UpdateHPBarWidget();
	virtual void ShowHPBar();
	virtual void HideHPBar();

	FTimerHandle HPBarUpdateTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|HPBar")
	float MinHPBarScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|HPBar")
	float MaxHPBarScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|HPBar")
	FVector DesiredScale;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HPWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HPWidgetComponent;
#pragma endregion
	
#pragma region Alert Icon Widget
public:
	UFUNCTION(BlueprintCallable, Category = "Monster|Alert")
	virtual void ShowAlertIcon();

	UFUNCTION(BlueprintCallable, Category = "Monster|Alert")
	virtual void HideAlertIcon();

	UFUNCTION(BlueprintPure, Category = "Monster|Alert")
	bool IsAlerting() const { return bIsAlerting; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster | Sounds")
	USoundCue* HowlSound;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Alert")
	UWidgetComponent* AlertWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Alert")
	TSubclassOf<UUserWidget> AlertWidgetClass;

	UPROPERTY()
	UUserWidget* AlertWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|Alert")
	bool bIsAlerting = false;

	// 위젯 표시 거리 제한
	UPROPERTY(EditDefaultsOnly, Category = "UI|Alert", meta = (ClampMin = "0"))
	float AlertWidgetCullDistance = 10000.0f;

#pragma endregion
	
#pragma region Stat Scaling (Player Level Based)
public:
	/**
	 * 플레이어 레벨 기반 몬스터 스탯 초기화
	 * BeginPlay 또는 스폰 시 호출 – 서브클래스에서 override 가능
	 *
	 *  공격력  = PlayerLevel × 3.14
	 *  방어력  = (PlayerLevel / 2) + GetMonsterTypeBaseDefense(MonsterType)
	 */
	virtual void InitializeStatsFromPlayerLevel(int32 PlayerLevel);

	/** 몬스터 종류별 기본 HP */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float GetMonsterTypeBaseHP(EMonsterType Type);

	/** 몬스터 종류별 레벨당 HP 증가량 */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float GetMonsterTypeHPPerLevel(EMonsterType Type);

	/** 최대 HP = BaseHP + (PlayerLevel × HPPerLevel) */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float CalculateMaxHealth(int32 PlayerLevel, EMonsterType Type);

	/** 몬스터 종류별 기본 방어력 (플레이어 레벨 보정값에 추가) */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float GetMonsterTypeBaseDefense(EMonsterType Type);

	/** 일반 공격 데미지 = PlayerLevel × 3.14 */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float CalculateAttackDamage(int32 PlayerLevel);

	/** 광역 공격 데미지 = (PlayerLevel × 3.14) × 1.5 */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float CalculateAoEDamage(int32 PlayerLevel);

	/** 방어력 = (PlayerLevel / 2) + 종류별 기본 방어력 */
	UFUNCTION(BlueprintPure, Category = "Monster|Scaling")
	static float CalculateDefense(int32 PlayerLevel, EMonsterType Type);

private:
	/** BeginPlay 에서 첫 번째 플레이어 레벨로 스탯 자동 초기화 */
	void AutoInitStatsFromNearestPlayer();
#pragma endregion

#pragma region Rotation System
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Rotation")
	bool bUseSmoothRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Rotation", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float RotationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Rotation", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float RotationUpdateInterval = 0.05f;

	void StartRotationUpdate();
	void StopRotationUpdate();

protected:
	void UpdateRotation();
    
	FTimerHandle RotationTimerHandle;
#pragma endregion
	
#pragma region Private
private:

	UPROPERTY()
	UUK_MonsterHealthBar* HPWidget;

	bool bHPVisible = false;
#pragma endregion
};