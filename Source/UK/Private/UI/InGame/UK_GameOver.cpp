// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_GameOver.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UUK_GameOver::NativeConstruct()
{
	Super::NativeConstruct();
}


void UUK_GameOver::SetupGameOverUI()
{
	if ( FadeIn )
	{
		PlayAnimation(FadeIn);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if ( PC )
	{
		bIsFocusable = true;

		PC->bShowMouseCursor = true;

		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputModeData);
	}

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}
