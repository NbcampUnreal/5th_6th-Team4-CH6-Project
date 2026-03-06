#include "AIMonster/Calculation/UK_MonsterDamageExecutionCalculation.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "Tags/UK_GameplayTags.h"

// ─────────────────────────────────────────────────────────────────────────────
// 캡처 구조체
//  · AttackPower : Source(플레이어) 의 공격력
//  · Defense     : Target(몬스터)  의 방어력
// ─────────────────────────────────────────────────────────────────────────────
struct FMonsterDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense)

	FMonsterDamageCapture()
	{
		// 플레이어 AttackPower – Source, Snapshot
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true
		);

		// 몬스터 Defense – Target, Snapshot
		DefenseDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_MonsterAttributeSet::GetDefenseAttribute(),
			EGameplayEffectAttributeCaptureSource::Target,
			true
		);
	}
};

static FMonsterDamageCapture& GetMonsterDamageCapture()
{
	static FMonsterDamageCapture MonsterDamageCapture;
	return MonsterDamageCapture;
}

// ─────────────────────────────────────────────────────────────────────────────

UUK_MonsterDamageExecutionCalculation::UUK_MonsterDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(GetMonsterDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetMonsterDamageCapture().DefenseDef);
}

void UUK_MonsterDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// ── 1. 플레이어 공격력 ────────────────────────────────────────────────
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetMonsterDamageCapture().AttackPowerDef, EvalParams, AttackPower);

	// ── 2. 스킬 데미지 퍼센트 (SetByCaller: Data.Damage) ────────────────────
	float SkillDamagePercent = Spec.GetSetByCallerMagnitude(
		UK_GameplayTags::Data::Damage,
		/*bWarnIfNotFound=*/false,
		0.f
	);

	// 퍼센트 → 배율 변환 (ex. 150 → 1.5)
	SkillDamagePercent /= 100.f;

	// ── 3. 방어 차감 전 원본 데미지 ─────────────────────────────────────
	//   RawDamage = PlayerAttackPower × SkillDamagePercent
	const float RawDamage = FMath::Max(AttackPower * SkillDamagePercent, 0.f);

	// ── 4. 몬스터 방어력 ─────────────────────────────────────────────────
	//   Defense = (PlayerLevel / 2) + 몬스터 종류별 기본 방어력
	//   → AAIMonsterBase::InitializeStatsFromPlayerLevel() 에서 설정됨
	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetMonsterDamageCapture().DefenseDef, EvalParams, Defense);
	Defense = FMath::Max(Defense, 0.f);

	// ── 5. 최종 데미지 = max(0, RawDamage - Defense) ─────────────────────
	const float FinalDamage = FMath::Max(RawDamage - Defense, 0.f);

	UE_LOG(LogTemp, Warning,
		TEXT("[MonsterDamageExec] ATK=%.1f × %.0f%% = Raw%.1f | DEF=%.1f | Final=%.1f"),
		AttackPower, SkillDamagePercent * 100.f, RawDamage, Defense, FinalDamage);

	if (FinalDamage > 0.f)
	{
		// 몬스터 AttributeSet 의 Damage 에 기록 → PostGameplayEffectExecute 에서 Health 차감
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UUK_MonsterAttributeSet::GetDamageAttribute(),
			EGameplayModOp::Additive,
			FinalDamage
		));
	}
}