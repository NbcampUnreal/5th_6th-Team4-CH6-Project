#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_EliteSpecialAttack.generated.h"

/**
 * 엘리트 몬스터 특수 공격 BT Task
 *
 * - 쿨다운이 남아있으면 즉시 Failed (BT 가 일반 공격 브랜치로 넘어감)
 * - 쿨다운이 돌아왔으면 특수 공격 몽타주 재생 → 완료 시 Succeeded
 * - 분노 상태(Phase2)에서는 AIEliteMonsterBase 가 쿨다운을 절반으로 처리
 */
UCLASS()
class UK_API UUK_BTTask_EliteSpecialAttack : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_EliteSpecialAttack();
#pragma endregion

#pragma region Execution
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Special Attack Callback
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Private
private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	void OnSpecialAttackFinished(bool bSucceeded);
#pragma endregion
};