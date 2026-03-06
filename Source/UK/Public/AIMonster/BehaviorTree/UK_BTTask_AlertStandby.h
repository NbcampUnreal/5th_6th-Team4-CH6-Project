#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_AlertStandby.generated.h"

/**
 * 플레이어 최초 감지 시 제자리에서 플레이어를 바라보며 AlertDuration(3초) 대기.
 * - 3초 후 → bAlertPhase 해제 → BT가 추격 브랜치로 진입
 * - 3초 이내 플레이어가 감지 범위 이탈 → AbortTask → TargetPlayer 클리어 → 추격 없음
 */
UCLASS()
class UK_API UUK_BTTask_AlertStandby : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_AlertStandby();
	virtual uint16 GetInstanceMemorySize() const override;
#pragma endregion

#pragma region Task Interface
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void                TickTask   (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask  (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Blackboard Keys
public:
	/** 추격 대상 (Object) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	/** Alert 중인지 여부 (Bool) — BB에 bAlertPhase 키 추가 필요 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PendingTargetKey;
#pragma endregion

#pragma region Settings
public:
	/** 느낌표를 띄우고 대기할 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.1"))
	float AlertDuration = 3.0f;

	/** 플레이어를 향해 회전하는 속도 */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "1.0"))
	float LookAtSpeed = 6.0f;
#pragma endregion

#pragma region Memory
private:
	struct FAlertStandbyMemory
	{
		float ElapsedTime = 0.f;
	};
#pragma endregion
};