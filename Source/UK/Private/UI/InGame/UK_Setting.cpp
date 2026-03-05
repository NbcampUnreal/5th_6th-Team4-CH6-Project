// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_Setting.h"
#include "UI/InGame/Setting/UK_Sound.h"
#include "UI/InGame/Setting/UK_Screen.h"

void UUK_Setting::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Sound )
	{
		Sound->OnClicked.AddDynamic(this, &UUK_Setting::OnSoundButtonClicked);
	}

	if ( Video )
	{
		Video->OnClicked.AddDynamic(this, &UUK_Setting::OnVideoButtonClicked);
	}

	if ( Control )
	{
		Control->OnClicked.AddDynamic(this, &UUK_Setting::OnControlButtonClicked);
	}
}

void UUK_Setting::OnSoundButtonClicked()
{
	if ( !SoundWidgetClass ) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if ( !PC ) return;

	UUK_Sound* SoundWidget = CreateWidget<UUK_Sound>(PC, SoundWidgetClass);
	if ( !SoundWidget ) return;

	RemoveFromParent();

	SoundWidget->AddToViewport();
}

void UUK_Setting::OnVideoButtonClicked()
{
	if ( !VideoWidgetClass ) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if ( !PC ) return;

	UUK_Screen* VideoWidget = CreateWidget<UUK_Screen>(PC, VideoWidgetClass);
	if ( !VideoWidget ) return;

	RemoveFromParent();

	VideoWidget->AddToViewport();
}

void UUK_Setting::OnControlButtonClicked()
{
	if ( !ControlWidgetClass ) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if ( !PC ) return;

	UUK_Screen* ControlWidget = CreateWidget<UUK_Screen>(PC, ControlWidgetClass);
	if ( !ControlWidget ) return;

	RemoveFromParent();

	ControlWidget->AddToViewport();
}
