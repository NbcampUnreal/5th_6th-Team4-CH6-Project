// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UK_SkillAttributeSet.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeDataChanged, float, OldValue, float, NewValue);
UCLASS()
class UK_API UUK_SkillAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UUK_SkillAttributeSet();

	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Cost);
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, CoolDown);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	UPROPERTY(BlueprintAssignable, Category = "Attribute")
	FAttributeDataChanged OnCostChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attribute")
	FAttributeDataChanged OnCoolDownChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Cost;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CoolDown;

};
