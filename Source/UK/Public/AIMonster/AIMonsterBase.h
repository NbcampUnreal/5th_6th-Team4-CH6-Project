#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIMonsterBase.generated.h"

class UBehaviorTree;

/* AI ���� ���� ������ ���� */
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

	/* ���� ���� ���¿��� ��û �� ���� (Controller�� ȣ�� �� ����)*/
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

	/* �ʱ� ���°� ���� */
	UPROPERTY(ReplicatedUsing = OnRep_MonsterState)
	EMonsterState CurrentState = EMonsterState::Idle;

	UFUNCTION()
	void OnRep_MonsterState();

	void SetServerState(EMonsterState NewState);

	/* AI Ȱ�� / ��Ȱ�� ����ġ ������ �ϰԵ� �� (Controller�� ȣ�� �� ����) */
public:
	void SetAIActive(bool bAcitve);

protected:
	/* ���� ���º� ������ �Լ��� ��ӹ��� �ڽ�Ŭ�������� override �� �Լ� */
	virtual void OnIdle();
	virtual void OnChase(float DeltaSeconds);
	virtual void OnPatrol();
	virtual void OnAttack();
	virtual void OnDead();

	/* �⺻ ����ȭ ���̽� (��ƴ� �ΰ� ���� ÷�� �ɼ� �ֽ��ϴ�) */
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	float TickIntervalPatrol = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	float TickIntervalChase = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	float TickIntervalAttack = 0.1f;

public:
	/* Replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
