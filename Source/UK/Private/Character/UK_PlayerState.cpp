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
	ASC->AddAttributeSetSubobject(StatusAttributeSet);
}

UAbilitySystemComponent* AUK_PlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

void AUK_PlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (ASC)
	{
		InitializeAttributes();
	}
}

void AUK_PlayerState::InitializeAttributes() const
{
	ASC->SetNumericAttributeBase(StatusAttributeSet->GetMaxHealthAttribute(), 200.f);
	ASC->SetNumericAttributeBase(StatusAttributeSet->GetHealthAttribute(), StatusAttributeSet->GetMaxHealth());
}
