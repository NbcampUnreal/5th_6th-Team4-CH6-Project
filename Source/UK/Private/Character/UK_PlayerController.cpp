// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController.h"
#include "EnhancedInputSubsystems.h"

AUK_PlayerController::AUK_PlayerController()
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
}


void AUK_PlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	//SetIgnoreMoveInput(false);
	//SetIgnoreLookInput(false);
}

void AUK_PlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
}