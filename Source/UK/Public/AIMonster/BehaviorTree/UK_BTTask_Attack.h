#pragma once

#include "CoreMinimal.h"
#include "AIMonster/BehaviorTree/UK_BTTask_AttackBase.h"
#include "UK_BTTask_Attack.generated.h"

/**
 * 일반 몬스터 공격 태스크
 */
UCLASS()
class UK_API UUK_BTTask_Attack : public UUK_BTTask_AttackBase
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_Attack();
#pragma endregion

#pragma region Attack Interface
protected:
	virtual bool PlayAttackMontage(AAIMonsterBase* Monster) override;
	virtual FSimpleDelegate& GetAttackDelegate(AAIMonsterBase* Monster) override;
#pragma endregion
};