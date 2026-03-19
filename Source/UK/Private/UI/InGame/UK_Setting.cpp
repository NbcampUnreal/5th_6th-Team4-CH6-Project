// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_Setting.h"
#include "UI/InGame/Setting/UK_Sound.h"
#include "UI/InGame/Setting/UK_Screen.h"
#include "UI/InGame/Setting/UK_Control.h"
#include "Character/UK_PlayerController.h"

void UUK_Setting::NativeConstruct()
{
	Super::NativeConstruct();


	UK_PC = Cast<AUK_PlayerController>(GetOwningPlayer());

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

	if ( Exit )
	{
		Exit->OnClicked.AddDynamic(this, &UUK_Setting::OnExitButtonClicked);
	}
}

void UUK_Setting::OnSoundButtonClicked()
{
	if ( !SoundWidgetClass ) return;

	if ( !UK_PC ) return;

	UUK_Sound* SoundWidget = CreateWidget<UUK_Sound>(UK_PC, SoundWidgetClass);
	if ( !SoundWidget ) return;

	SoundWidget->SetParentWidget(this);

	SoundWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(SoundWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	UK_PC->SetInputMode(InputMode);
	UK_PC->bShowMouseCursor = true;

	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UUK_Setting::OnVideoButtonClicked()
{
	if ( !VideoWidgetClass ) return;

	if ( !UK_PC ) return;

	UUK_Screen* VideoWidget = CreateWidget<UUK_Screen>(UK_PC, VideoWidgetClass);
	if ( !VideoWidget ) return;

	VideoWidget->SetParentWidget(this);

	VideoWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(VideoWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	UK_PC->SetInputMode(InputMode);
	UK_PC->bShowMouseCursor = true;

	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UUK_Setting::OnControlButtonClicked()
{
	if ( !ControlWidgetClass ) return;

	if ( !UK_PC ) return;

	UUK_Control* ControlWidget = CreateWidget<UUK_Control>(UK_PC, ControlWidgetClass);
	if ( !ControlWidget )
	{
		UE_LOG(LogTemp, Error, TEXT("SoundWidgetClass is NOT assigned in Blueprint!"));
	}

	ControlWidget->SetParentWidget(this);

	ControlWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ControlWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	UK_PC->SetInputMode(InputMode);
	UK_PC->bShowMouseCursor = true;

	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UUK_Setting::OnExitButtonClicked()
{
	if ( UK_PC )
	{
		UK_PC->Setting_UI();
	}
	else
	{
		RemoveFromParent();
	}

}