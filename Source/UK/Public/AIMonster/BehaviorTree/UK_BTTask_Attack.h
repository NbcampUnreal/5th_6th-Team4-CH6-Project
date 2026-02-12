#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_Attack.generated.h"

/**
 * 몬스터 공격 BT Task
 * 랜덤 공격 몽타주를 재생하고, 종료될 때까지 InProgress.
 * 실제 데미지는 AnimNotifyState_MonsterMeleeTrace에서 트레이스 처리.
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
};