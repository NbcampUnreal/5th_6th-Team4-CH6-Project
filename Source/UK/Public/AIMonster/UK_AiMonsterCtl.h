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
	AActor* GetCurrentTarget() const { return CurrentTarget; }
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
private:

	/* 컨트롤 중인 몬스터 */
	UPROPERTY()
	AAIMonsterBase* ControlledMonster;

	UPROPERTY()
	AActor* CurrentTarget;

	/* AI 판단 */

	void UpdateTarget();
	void UpdateState();
	void HandleMovement();

	/* 설정값 */
	/* 밸런스 테스트를 통해 수치 변경 필요 */

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

	/* 디버그용 드로우 (삭제예정) */

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug = true;

	void DrawAIDebug() const;
};




