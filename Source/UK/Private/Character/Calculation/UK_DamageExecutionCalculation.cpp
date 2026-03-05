// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Calculation/UK_DamageExecutionCalculation.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"

struct FDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)

	FDamageCapture()
	{
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true
		);

		//DEFINE_ATTRIBUTE_CAPTUREDEF(UUK_PlayerStatusAttributeSet, AttackPower, Source, false);
	}
};

static FDamageCapture& GetDamageCapture()
{
	static FDamageCapture DamageCapture;
	return DamageCapture;
}

UUK_DamageExecutionCalculation::UUK_DamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(GetDamageCapture().AttackPowerDef);
}

void UUK_DamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//캐릭터의 기본 공격력
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().AttackPowerDef, EvalParams, AttackPower);

	// 스킬의 데미지 퍼센트
	float SkillDamagePercent = Spec.GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag("Data.Damage"),
		/*bWarnIfNotFound=*/false,
		0.f
	);
	
	// 퍼센트로 변환  
	SkillDamagePercent /= 100;
	
	//방어력 계산 전 최종데미지
	float FinalDamage = FMath::Max(AttackPower * SkillDamagePercent, 0.0f);

	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UUK_PlayerStatusAttributeSet::GetDamageAttribute()/*데미지를 AttributeSet에 전달*/,
			EGameplayModOp::Additive,
			FinalDamage
		));
	}
}
