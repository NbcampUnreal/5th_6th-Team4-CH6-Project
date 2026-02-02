#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/AI_MonsterStatComponent.h"
#include "AIMonsterBase.generated.h"

class UBehaviorTree;

/* AI 상태 관련 정의한 내용 */
UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle,
	Patrol,
	Chase,
	Attack,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, class AAIMonsterBase*, DeadMonster);

UCLASS(Abstract)
class UK_API AAIMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAIMonsterBase();

	/* 서버 전용 상태에서 요청 할 내용 (Controller가 호출 할 예정)*/
	UFUNCTION(Server, Reliable)
	void RequestState(EMonsterState NewState);

	UFUNCTION(BlueprintPure)
	EMonsterState GetCurrentState() const { return CurrentState; }
	
#pragma region Spawner System
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY()
	class AUK_MonsterSpawner* OwningSpawner;

	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	FVector SpawnLocation;

	UPROPERTY(BlueprintAssignable, Category = "Monster")
	FOnMonsterDeath OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void Die();

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsDead() const { return CurrentState == EMonsterState::Dead; }

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
	
#pragma endregion
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAI_MonsterStatComponent* StatComponent;

	/* 초기 상태값 설정 */
	UPROPERTY(ReplicatedUsing = OnRep_MonsterState)
	EMonsterState CurrentState = EMonsterState::Idle;

	UFUNCTION()
	void OnRep_MonsterState();

	void SetServerState(EMonsterState NewState);

	/* AI 활성 / 비활성 스위치 역할을 하게될 값 (Controller가 호출 할 예정) */
public:
	void SetAIActive(bool bAcitve);

protected:
	/* 현재 상태별 실행할 함수들 상속받은 자식클래스에서 override 될 함수 */
	virtual void OnIdle();
	virtual void OnChase(float DeltaSeconds);
	virtual void OnPatrol();
	virtual void OnAttack();
	virtual void OnDead();

	/* 기본 최적화 베이스 (깔아는 두고 수정 첨삭 될수 있습니다) */
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	float TickIntervalPatrol = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	float TickIntervalChase = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	float TickIntervalAttack = 0.1f;

public:

	void ReceiveDamage(float Damage);

	/* Replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
