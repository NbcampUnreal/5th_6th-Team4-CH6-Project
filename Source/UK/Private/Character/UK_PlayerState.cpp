// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"

AUK_PlayerState::AUK_PlayerState()
{
	SetNetUpdateFrequency(100.f);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	StatusAttributeSet = CreateDefaultSubobject<UUK_PlayerStatusAttributeSet>(TEXT("PlayerStatusAttributeSet"));
}

UAbilitySystemComponent* AUK_PlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}
