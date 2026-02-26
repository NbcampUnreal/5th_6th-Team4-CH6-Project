// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_Setting.h"
#include "UI/InGame/Setting/UK_Sound.h"

void UUK_Setting::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Sound )
	{
		Sound->OnClicked.AddDynamic(this, &UUK_Setting::OnSoundButtonClicked);
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
