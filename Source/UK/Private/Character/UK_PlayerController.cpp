// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController.h"
#include "EnhancedInputSubsystems.h"

AUK_PlayerController::AUK_PlayerController() :
	CharacterIMC(nullptr),
	AbilitiesIMC(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr),
	JumpAction(nullptr),
	SprintAction(nullptr),
	AttackAction(nullptr),
	ZoomIn(nullptr),
	ZoomOut(nullptr)
{
}
void AUK_PlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMax = 45.f; // ¾Æ·¡
		PlayerCameraManager->ViewPitchMin = -60.f; // À§
	}
}

void AUK_PlayerController::BeginPlay()
{
	Super::BeginPlay();
}


void AUK_PlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
	if (TObjectPtr<ULocalPlayer> LocalPlayer = GetLocalPlayer())
	{
		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> SupSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (CharacterIMC)
			{
				SupSystem->AddMappingContext(CharacterIMC, 0);
			}

			if (AbilitiesIMC)
			{
				SupSystem->AddMappingContext(AbilitiesIMC, 1);
			}
		}
	}
}