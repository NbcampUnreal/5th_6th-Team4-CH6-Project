// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "UK_SkillAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_SkillAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UUK_SkillAttributeSet();
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Cost;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CoolDown;

};
