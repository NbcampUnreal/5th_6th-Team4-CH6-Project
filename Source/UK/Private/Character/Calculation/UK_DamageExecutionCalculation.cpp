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

	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().AttackPowerDef, EvalParams, AttackPower);

	float BaseDamage = Spec.GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag("Data.Damage.NomalSkill"),
		/*bWarnIfNotFound=*/false,
		0.f
	);

	float FinalDamage = FMath::Max(BaseDamage + AttackPower, 0.0f);

	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UUK_PlayerStatusAttributeSet::GetDamageAttribute(),
			EGameplayModOp::Additive,
			FinalDamage
		));
	}
}
