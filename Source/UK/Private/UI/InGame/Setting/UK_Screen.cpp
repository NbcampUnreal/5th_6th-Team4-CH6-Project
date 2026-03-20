// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/Setting/UK_Screen.h"

void UUK_Screen::NativeConstruct()
{
	Super::NativeConstruct();
	if ( ScreenBackButton )
		ScreenBackButton->OnClicked.AddDynamic(this, &UUK_Screen::OnBackButtonClicked);
}

void UUK_Screen::OnBackButtonClicked()
{
	if ( ParentSettingWidget )
	{
		ParentSettingWidget->SetVisibility(ESlateVisibility::Visible);

		if ( APlayerController* PC = GetOwningPlayer() )
		{
			FInputModeGameAndUI Mode;

			Mode.SetWidgetToFocus(ParentSettingWidget->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			Mode.SetHideCursorDuringCapture(false);

			PC->SetInputMode(Mode);
		}

		RemoveFromParent();
	}
}