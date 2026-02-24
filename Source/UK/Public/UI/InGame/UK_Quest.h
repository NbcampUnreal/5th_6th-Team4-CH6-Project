// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "UK_Quest.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_Quest : public UUserWidget
{
	GENERATED_BODY()
public:
	UUK_Quest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

private:

	UPROPERTY(meta = ( BindWidget ))
	UButton* Accept_Button;

	UPROPERTY(meta = ( BindWidget ))
	UButton* Exit_Button;

};
