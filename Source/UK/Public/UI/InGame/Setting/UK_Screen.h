// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "UK_Screen.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_Screen : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY()
	class UUserWidget* ParentSettingWidget;
public:
	void SetParentWidget(UUserWidget* InParent) { ParentSettingWidget = InParent; }

	UPROPERTY(meta = ( BindWidget ))
	class UButton* ScreenBackButton;

	UFUNCTION()
	void OnBackButtonClicked();

};
