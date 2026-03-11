#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_AttackBase.generated.h"

class AAIMonsterBase;

/**
 * 몬스터 공격 태스크 베이스 클래스
 * - 일반 공격(Attack)과 특수 공격(EliteSpecialAttack) 공통 로직 처리
 * - 자식 클래스는 몽타주 재생 로직만 구현
 */
UCLASS(Abstract)
class UK_API UUK_BTTask_AttackBase : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_AttackBase();
#pragma endregion

#pragma region Execution
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override final;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override final;
#pragma endregion

#pragma region Attack Interface (자식이 구현)
protected:
	virtual bool PlayAttackMontage(AAIMonsterBase* Monster) PURE_VIRTUAL(UUK_BTTask_AttackBase::PlayAttackMontage, return false;);
	
	virtual FSimpleDelegate& GetAttackDelegate(AAIMonsterBase* Monster) PURE_VIRTUAL(UUK_BTTask_AttackBase::GetAttackDelegate, static FSimpleDelegate Dummy; return Dummy;);
#pragma endregion

#pragma region Private
private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	void OnAttackComplete();  
	void StopMovementAndRotateToTarget(AAIMonsterBase* Monster, UBehaviorTreeComponent& OwnerComp);
	void CleanupAttack(AAIMonsterBase* Monster);
#pragma endregion
};