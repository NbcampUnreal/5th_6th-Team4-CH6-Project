// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "UK_Control.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_Control : public UUserWidget
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
	class UButton* ControlBackButton;

	UFUNCTION()
	void OnBackButtonClicked();

	void StartRebind(FName MappingName);

	FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	void ApplyRebind(FName MappingName, FKey NewKey);
protected:
	FName PendingMapping;
	bool bListeningForKey = false;
	
};
