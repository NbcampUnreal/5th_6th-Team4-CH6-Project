// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Calculation/UK_DamageExecutionCalculation.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"


struct FDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CurrentPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage)
	

	FDamageCapture()
	{
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			false
			);					
		CurrentPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UUK_PlayerStatusAttributeSet::GetCurrentPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			false
			);				
		CriticalChanceDef = FGameplayEffectAttributeCaptureDefinition(
				UUK_PlayerStatusAttributeSet::GetCriticalChanceAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				false
			);		
		CriticalDamageDef = FGameplayEffectAttributeCaptureDefinition(
				UUK_PlayerStatusAttributeSet::GetCriticalDamageAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				false
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
	RelevantAttributesToCapture.Add(GetDamageCapture().CurrentPowerDef);
	RelevantAttributesToCapture.Add(GetDamageCapture().CriticalChanceDef);
	RelevantAttributesToCapture.Add(GetDamageCapture().CriticalDamageDef);
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
	
	//무기 공격력
	float CurrentPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().CurrentPowerDef, EvalParams, CurrentPower);	
	
	UE_LOG(LogTemp, Display, TEXT("AttackPower : %f, CurrentPower : %f"),AttackPower ,  CurrentPower);
	
	// 기본 공격력 + 무기 공격력
	float FinalAttack = AttackPower + CurrentPower;
	
	//크리티컬 확률
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().CriticalChanceDef, EvalParams, CriticalChance);
	
	float RandomValue = FMath::FRandRange(0.f, 100.f);
	
	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		FDamageCapture().CriticalDamageDef, EvalParams, CriticalDamage);
	CriticalDamage /= 100;
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
	float FinalDamage = FMath::Max(FinalAttack * SkillDamagePercent, 0.0f);
	if (RandomValue <= CriticalChance)
	{
		FinalDamage = FMath::Max(FinalDamage * CriticalDamage, 0.f);
		UE_LOG(LogTemp, Display, TEXT("FinalDamage : %f"), FinalDamage);
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
