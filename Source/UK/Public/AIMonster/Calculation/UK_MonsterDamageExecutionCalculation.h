#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "UK_MonsterDamageExecutionCalculation.generated.h"

/**
 * 몬스터가 받는 데미지 계산 Execution
 *
 * ■ Source : 플레이어 (UUK_PlayerStatusAttributeSet::AttackPower 캡처)
 * ■ Target : 몬스터 (UUK_MonsterAttributeSet::Defense 캡처)
 *
 * 계산 공식
 *   RawDamage   = PlayerAttackPower × (SkillDamagePercent / 100)
 *   FinalDamage = max(0, RawDamage - MonsterDefense)
 *
 * 출력 Attribute
 *   UUK_MonsterAttributeSet::Damage  (PostGameplayEffectExecute 에서 Health 차감)
 */
UCLASS()
class UK_API UUK_MonsterDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UUK_MonsterDamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};