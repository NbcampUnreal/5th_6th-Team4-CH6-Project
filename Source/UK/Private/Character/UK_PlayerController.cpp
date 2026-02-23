// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController.h"
#include "EnhancedInputSubsystems.h"

AUK_PlayerController::AUK_PlayerController() : bMouseCursorEnabled(false)
{

}
void AUK_PlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMax = 45.f; // 아래
		PlayerCameraManager->ViewPitchMin = -60.f; // 위
	}
}

void AUK_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	DisableMouseCursorMode();
}


void AUK_PlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;

	DisableMouseCursorMode();
}

void AUK_PlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
}


void AUK_PlayerController::EnableMouseCursorMode()
{
	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);

	SetIgnoreLookInput(true);

	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AUK_PlayerController::DisableMouseCursorMode()
{
	bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	SetIgnoreLookInput(false);
}

void AUK_PlayerController::ToggleMouseCursor()
{
	bMouseCursorEnabled = !bMouseCursorEnabled;

	if (bMouseCursorEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnableMouseCursorMode()"));
		EnableMouseCursorMode();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DisableMouseCursorMode()"));
		DisableMouseCursorMode();
	}
}