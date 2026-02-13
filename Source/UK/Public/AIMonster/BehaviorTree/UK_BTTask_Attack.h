#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_Attack.generated.h"

/**
 * 몬스터 공격 BT Task
 */
UCLASS()
class UK_API UUK_BTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUK_BTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "Attack")
	float FacingAngleTolerance = 30.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float MaxFacingWaitTime = 1.0f;
	
private:
	/** 몽타주가 이미 시작됐는지 */
	bool bMontageStarted = false;

	/** 회전 대기 누적 시간 */
	float FacingWaitElapsed = 0.f;
};