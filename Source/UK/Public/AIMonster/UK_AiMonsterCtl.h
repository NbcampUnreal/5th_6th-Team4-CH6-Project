#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "UK_AiMonsterCtl.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

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
	UPROPERTY()
	AAIMonsterBase* ControlledMonster;

	UPROPERTY()
	AActor* CurrentTarget;

	// 이벤트 기반 탐지
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	// 감지/소실 이벤트 콜백
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// BT 미사용 시
	void UpdateState();
	void HandleMovement();
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
	float ControllerTickInterval = 0.5f;

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

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug = true;

	void DrawAIDebug() const;
};