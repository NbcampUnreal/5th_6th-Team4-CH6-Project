#pragma once

#include "CoreMinimal.h"
#include "AIMonster/BehaviorTree/UK_BTTask_AttackBase.h"
#include "UK_BTTask_EliteSpecialAttack.generated.h"

/**
 * 엘리트 몬스터 특수 공격 태스크
 */
UCLASS()
class UK_API UUK_BTTask_EliteSpecialAttack : public UUK_BTTask_AttackBase
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_EliteSpecialAttack();
#pragma endregion

#pragma region Attack Interface
protected:
	virtual bool PlayAttackMontage(AAIMonsterBase* Monster) override;
	virtual FSimpleDelegate& GetAttackDelegate(AAIMonsterBase* Monster) override;
#pragma endregion
};