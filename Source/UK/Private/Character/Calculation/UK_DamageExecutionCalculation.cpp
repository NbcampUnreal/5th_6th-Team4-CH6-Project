// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Calculation/UK_DamageExecutionCalculation.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"


struct FDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage)
	

	FDamageCapture()
	{
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true
			);				
		CriticalChanceDef = FGameplayEffectAttributeCaptureDefinition(
				UUK_PlayerStatusAttributeSet::GetCriticalChanceAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				true
			);		
		CriticalDamageDef = FGameplayEffectAttributeCaptureDefinition(
				UUK_PlayerStatusAttributeSet::GetCriticalDamageAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				true
			);		

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
	
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().CriticalChanceDef, EvalParams, CriticalChance);
	
	float RandomValue = FMath::FRandRange(0.f, 100.f);
	
	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().CriticalDamageDef, EvalParams, CriticalDamage);
	CriticalDamage += 1.f;
	
	// 스킬의 데미지 퍼센트
	float SkillDamagePercent = Spec.GetSetByCallerMagnitude(
		DamageTag,
		/*bWarnIfNotFound=*/false,
		0.f
	);
	
	// 퍼센트로 변환  
	SkillDamagePercent /= 100;
	
	
	/*최종 피해량 =(캐릭터 공격력) X (스킬 계수) X (치명타 피해량) */
	/*FinalDamage = AttackPower * SkillDamagePercent * CriticalDamage */
	//방어력 계산 전 최종데미지
	float FinalDamage = FMath::Max(AttackPower * SkillDamagePercent, 0.0f);
	if (RandomValue <= CriticalChance)
	{
		FinalDamage = FMath::Max(FinalDamage * CriticalDamage, 0.f);
	}

	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UUK_MonsterAttributeSet::GetDamageAttribute()/*데미지를 AttributeSet에 전달*/,
			EGameplayModOp::Additive,
			FinalDamage
		));
	}
}
