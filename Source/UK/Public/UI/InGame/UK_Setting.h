// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "UK_Setting.generated.h"


class AUK_PlayerController;
class UUK_Sound;
class UUK_Screen;
class UUK_Control;
class UWidgetSwitcher;
/**
 * 
 */
UCLASS()
class UK_API UUK_Setting : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	void ClearAllWidgets();

	void SetInputConfig();

	UPROPERTY(BlueprintReadWrite, meta = ( BindWidget ))
	UWidgetSwitcher* SettingSwitcher;

	UPROPERTY(meta = ( BindWidget ))
	UButton* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> SoundWidgetClass;

	UPROPERTY(meta = ( BindWidget ))
	UButton* Control;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> ControlWidgetClass;

	UPROPERTY(meta = ( BindWidget ))
	UButton* Video;

	UPROPERTY(meta = ( BindWidget ))
	UButton* Infomation;

	UPROPERTY(meta = ( BindWidget ))
	UButton* Exit;

	UFUNCTION()
	void OnSoundButtonClicked();

	UFUNCTION()
	void OnVideoButtonClicked();

	UFUNCTION()
	void OnControlButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

protected:
	UPROPERTY()
	class AUK_PlayerController* UK_PC;

};
