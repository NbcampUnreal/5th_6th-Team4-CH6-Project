// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_Crafting.generated.h"

class UUK_MoneyWidget;
/**
 * 
 */
UCLASS()
class UK_API UUK_Crafting : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = ( BindWidget ))
	class UUK_MoneyWidget* MoneyWidget;
protected:
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
};
