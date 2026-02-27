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

#pragma region Initialization
public:
	UUK_BTTask_Attack();
#pragma endregion

#pragma region Execution
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Attack Callback
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Private
private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	void OnAttackFinished(bool bSucceeded);
#pragma endregion
};