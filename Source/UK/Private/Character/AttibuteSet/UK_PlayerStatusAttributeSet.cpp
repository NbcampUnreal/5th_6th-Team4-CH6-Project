// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/UK_CharacterBase.h"

UUK_PlayerStatusAttributeSet::UUK_PlayerStatusAttributeSet()
{
}

void UUK_PlayerStatusAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		
		SetDamage(0.0f);

		if (LocalDamage > 0.0f)
		{
			const float OldHealth = GetHealth();
			const float NewHealth = FMath::Max(0.0f, GetHealth() - LocalDamage);
			SetHealth(NewHealth);
			
			UE_LOG(LogTemp, Warning, TEXT(" Health Updated: %.1f → %.1f (Damage: %.1f)"), 
				OldHealth, NewHealth, LocalDamage);
		}
	}
	
	else if (Attribute == GetHealthAttribute())
	{
		const float OldValue = NewValue;
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
		
		if (OldValue != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("[AttributeSet] PreAttributeChange - Health clamped: %.1f → %.1f"), OldValue, NewValue);
		}
		
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		UE_LOG(LogTemp, Log, TEXT("   Health: %.1f → %.1f (Max: %.1f)"), 
			OldValue, GetHealth(), GetMaxHealth());
		
		if (GetHealth() <= 0.0f)
		{
			HandleOutOfHealth();
		}
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		const float OldMaxHealth = GetMaxHealth();
		
		SetHealth(GetMaxHealth());
		
		if (OldMaxHealth != GetHealth())
		{
			UE_LOG(LogTemp, Log, TEXT("   Health adjusted: %.1f → %.1f"), OldMaxHealth, GetMaxHealth());
		}
		
	}
}

void UUK_PlayerStatusAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	if (Attribute == GetDamageAttribute())
	{
		DamageChanged.Broadcast(OldValue, NewValue);
		SetDamage(0.f);
	}
	else if (Attribute == GetHealthAttribute())
	{
		if (GetHealth() <= 0.0f)
		{
			HandleOutOfHealth();
		}
		HealthChanged.Broadcast(OldValue, NewValue);
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		MaxHealthChanged.Broadcast(OldValue, NewValue);
	}
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UUK_PlayerStatusAttributeSet::HandleOutOfHealth()
{
	if (AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwningActor()))
	{
		Player->Dead();
	}
}
