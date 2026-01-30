#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "UK_AiMonsterCtl.generated.h"

UCLASS()
class UK_API AUK_AiMonsterCtl : public AAIController
{
	GENERATED_BODY()
	
public:
	AUK_AiMonsterCtl();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
private:
	/* ��Ʈ�� ���� ���� */

	UPROPERTY()
	AAIMonsterBase* ControlledMonster;

	/* ���� Ÿ�� */

	UPROPERTY()
	AActor* CurrentTarget;

	/*  AI �Ǵ�  */

	void UpdateTarget();
	void UpdateState();
	void HandleMovement();

	/* ������ */
	/* �ӽ÷� ���� ���� �� (���� �׽�Ʈ ���ؼ� ���� �ʿ�) */

	void SetNewPatrolTarget();

	FVector PatrolTarget;
	bool bHasPatrolTarget = false;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SearchRadius = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float ChaseRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float PatrolRadius = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float ControllerTickInterval = 0.2f;
	
#pragma region RVO
	UPROPERTY(EditAnywhere, Category = "AI|RVO")
	bool bUseRVOAvoidance = true;

	UPROPERTY(EditAnywhere, Category = "AI|RVO")
	int32 AvoidanceGroup = 1;

	UPROPERTY(EditAnywhere, Category = "AI|RVO")
	int32 GroupsToAvoid = 1;

	UPROPERTY(EditAnywhere, Category = "AI|RVO")
	int32 GroupsToIgnore = 0;
#pragma endregion

	/* ����� (���� ����) */

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug = true;

	void DrawAIDebug() const;
};




