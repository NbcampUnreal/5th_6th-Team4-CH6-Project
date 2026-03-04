// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "UK_PlayerStatusAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_PlayerStatusAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UUK_PlayerStatusAttributeSet();

	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, Health);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_PlayerStatusAttributeSet, Damage);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Health;
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxHealth;
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData AttackPower;
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Damage;
};
