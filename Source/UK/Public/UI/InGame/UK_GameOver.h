// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_GameOver.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class UK_API UUK_GameOver : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnRestartButtonClicked();

	UFUNCTION()
	void OnReExitButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = ( AllowPrivateAccess, BindWidget ))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = ( AllowPrivateAccess, BindWidget ))
	TObjectPtr<UButton> ExitButton;
};
