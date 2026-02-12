// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_Out_MainMenu.generated.h"

class UButton;
class UEditableText;
/**
 * 
 */
UCLASS()
class UK_API UUK_Out_MainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UUK_Out_MainMenu(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = ( AllowPrivateAccess, BindWidget ))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = ( AllowPrivateAccess, BindWidget ))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USLobbyLevelUI, Meta = ( AllowPrivateAccess, BindWidget ))
	TObjectPtr<UEditableText> ServerIPEditableText;
	
};
