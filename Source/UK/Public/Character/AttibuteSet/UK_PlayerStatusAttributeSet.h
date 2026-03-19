// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UK_PlayerStatusAttributeSet.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeDataChanged, float, OldValue, float, NewValue);
UCLASS()
class UK_API UUK_PlayerStatusAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UUK_PlayerStatusAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)override;
	
	void ExportStats(struct FCharacterStatSaveData& OutData);
	void ImportStats(const struct FCharacterStatSaveData& InData);
	void HandleOutOfHealth();
	
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, Health);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, CurrentPower);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, MaxMp);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, CurrentMp);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, CurrentStamina);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, MaxEXP);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, EXP);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, MaxLevel);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, Level);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, CriticalChance);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, CriticalDamage);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, Defence);

	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged MaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged HealthChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged AttackPowerChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged CurrentPowerChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged DamageChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged MaxMpChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged CurrentMpChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged MaxStaminaChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged CurrentStaminaChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged MaxEXPChanged;	
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged EXPChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged MaxLevelChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged LevelChanged;
		
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged CriticalChanceChanged;
		
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged CriticalDamageChanged;
	
	UPROPERTY(BlueprintAssignable,BlueprintReadOnly, Category = "Attribute")
	FAttributeDataChanged DefenceChanged;
	
protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData AttackPower;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentPower;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Damage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxMp;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentMp;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxStamina;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentStamina;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxEXP;	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData EXP;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxLevel;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Level;	
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CriticalChance;	
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CriticalDamage;	
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Defence;
};
