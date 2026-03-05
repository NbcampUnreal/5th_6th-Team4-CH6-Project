// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "UK_Setting.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_Setting : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> VideoWidgetClass;

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

};
