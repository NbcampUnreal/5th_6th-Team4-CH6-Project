#include "AIMonster/Calculation/UK_MonsterDamageExecutionCalculation.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "Tags/UK_GameplayTags.h"

// ─────────────────────────────────────────────────────────────────────────────
// 캡처 구조체
//
//  Source (플레이어)
//   · AttackPower   : 공격력
//   · CriticalChance: 치명타 확률 (0~100)
//   · CriticalDamage: 치명타 추가 배율 (ex. 0.5 = +50%)
//   · Level         : 플레이어 레벨 → 몬스터 방어력 계산에 사용
//
//  Target (몬스터)
//   · Defense       : 방어력 (= PlayerLevel/2 + 종류별 기본값)
// ─────────────────────────────────────────────────────────────────────────────
struct FMonsterDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Level)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense)

	FMonsterDamageCapture()
	{
		// ── Source : 플레이어 ────────────────────────────────────────────
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);

		CriticalChanceDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetCriticalChanceAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);

		CriticalDamageDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetCriticalDamageAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);

		LevelDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetLevelAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);

		// ── Target : 몬스터 ─────────────────────────────────────────────
		DefenseDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_MonsterAttributeSet::GetDefenseAttribute(),
			EGameplayEffectAttributeCaptureSource::Target, true);
	}
};

static FMonsterDamageCapture& GetMonsterDamageCapture()
{
	static FMonsterDamageCapture Capture;
	return Capture;
}

// ─────────────────────────────────────────────────────────────────────────────

UUK_MonsterDamageExecutionCalculation::UUK_MonsterDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(GetMonsterDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetMonsterDamageCapture().LevelDef);
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

	// ── 3. 스킬 데미지 퍼센트 (SetByCaller: Data.Damage) ─────────────────
	float SkillDamagePercent = Spec.GetSetByCallerMagnitude(
		UK_GameplayTags::Data::Damage,
		/*bWarnIfNotFound=*/false,
		0.f);
	SkillDamagePercent /= 100.f;

	// ── 4. 플레이어 레벨 (몬스터 방어력 갱신용) ──────────────────────────
	//   Defense 는 BeginPlay 에서 이미 설정되어 있으므로
	//   Level 은 참고용 로그에만 사용 (필요 시 동적 방어력 재계산 가능)
	float PlayerLevel = 1.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetMonsterDamageCapture().LevelDef, EvalParams, PlayerLevel);
	PlayerLevel = FMath::Max(PlayerLevel, 1.f);

	// ── 5. 원본 데미지 = 공격력 × 스킬 계수 ─────────────────────────────
	float RawDamage = FMath::Max(AttackPower * SkillDamagePercent, 0.f);

	// ── 7. 몬스터 방어력 차감 ────────────────────────────────────────────
	//   Defense = (PlayerLevel / 2) + 종류별 기본 방어력
	//   → AAIMonsterBase::InitializeStatsFromPlayerLevel() 에서 설정됨
	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetMonsterDamageCapture().DefenseDef, EvalParams, Defense);
	Defense = FMath::Max(Defense, 0.f);

	// ── 8. 최종 데미지 ────────────────────────────────────────────────────
	//   FinalDamage = max(0, RawDamage - Defense)
	const float FinalDamage = FMath::Max(RawDamage / (1.f + Defense), 0.f);

	if (FinalDamage > 0.f)
	{
		// 몬스터 AttributeSet 의 Damage 에 기록 → PostGameplayEffectExecute 에서 Health 차감
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UUK_MonsterAttributeSet::GetDamageAttribute(),
			EGameplayModOp::Additive,
			FinalDamage
		));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[DmgCalc] ATK=%.1f | SkillPct=%.2f | RawDmg=%.1f | Defense=%.1f | FinalDmg=%.1f"),
	AttackPower, SkillDamagePercent, RawDamage, Defense, FinalDamage);
}