// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "UK_PlayerState.generated.h"

#pragma region Forward Declaration
class UAbilitySystemComponent;
class UUK_PlayerStatusAttributeSet;
#pragma endregion

UCLASS()
class UK_API AUK_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AUK_PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void BeginPlay() override;
	void InitializeAttributes() const;
	UPROPERTY(VisibleAnywhere, Category = "GB|Abilities")
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GB|Abilities")
	UUK_PlayerStatusAttributeSet* StatusAttributeSet;
};
