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
	/* 컨트롤 중인 몬스터 */

	UPROPERTY()
	AAIMonsterBase* ControlledMonster;

	/* 현재 타겟 */

	UPROPERTY()
	AActor* CurrentTarget;

	/*  AI 판단  */

	void UpdateTarget();
	void UpdateState();
	void HandleMovement();

	/* 설정값 */
	/* 임시로 넣은 설정 값 (추후 테스트 통해서 변경 필요) */

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

	/* 디버깅 (삭제 예정) */

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug = true;

	void DrawAIDebug() const;
};




