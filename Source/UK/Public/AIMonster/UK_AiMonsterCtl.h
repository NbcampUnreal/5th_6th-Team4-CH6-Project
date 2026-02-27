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

#pragma region Initialization
public:
	AUK_AiMonsterCtl();
#pragma endregion

#pragma region Possess / UnPossess
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
#pragma endregion

#pragma region Player Detection
public:
	AActor* GetCurrentTarget() const { return CurrentTarget; }

private:
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	bool IsPlayerCharacter(AActor* Actor) const;

	UPROPERTY()
	AActor* CurrentTarget;
#pragma endregion

#pragma region State Management
private:
	UPROPERTY()
	AAIMonsterBase* ControlledMonster;

	void UpdateState();
	void UpdateFocusOnTarget(AActor* NewTarget);
#pragma endregion

#pragma region Movement
protected:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	void HandleMovement();
	void SetNewPatrolTarget();

	FVector PatrolTarget;
	bool    bHasPatrolTarget = false;
#pragma endregion

#pragma region AI Settings
private:
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
#pragma endregion

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
};