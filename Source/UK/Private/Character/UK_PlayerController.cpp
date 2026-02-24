// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController.h"
#include "EnhancedInputSubsystems.h"

AUK_PlayerController::AUK_PlayerController() 
	: bMouseCursorEnabled(false)
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
	StaminaWidget = CreateWidget<UUK_Stamina>(this, StaminaWidgetClass);
	StaminaWidget->AddToViewport();
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
	ConnectStaminaWidget();
}

void AUK_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if ( !StaminaWidget ) return;

	APawn* MyPawn = GetPawn();
	if ( !MyPawn )
	{
		StaminaWidget->SetTrackingPosition(FVector2D::ZeroVector, 1.f, false);
		return;
	}

	FVector CamLoc = PlayerCameraManager->GetCameraLocation();
	FRotator CamRot = PlayerCameraManager->GetCameraRotation();

	FVector CamRight = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
	FVector CamForward = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);

	FVector BaseLocation = MyPawn->GetActorLocation() + Stemina_Location;
	FVector TargetWorldPos = BaseLocation + ( CamRight * SideDistance );

	FVector2D ScreenPos;
	bool bProjected = ProjectWorldLocationToScreen(TargetWorldPos, ScreenPos, true);

	if ( !bProjected )
	{
		StaminaWidget->SetTrackingPosition(ScreenPos, 1.f, false);
		return;
	}

	FVector ToTarget = ( TargetWorldPos - CamLoc ).GetSafeNormal();
	float Dot = FVector::DotProduct(CamForward, ToTarget);

	if ( Dot < 0.15f )
	{
		StaminaWidget->SetTrackingPosition(ScreenPos, 1.f, false);
		return;
	}

	float Dist = FVector::Dist(CamLoc, TargetWorldPos);

	float Scale = FMath::GetMappedRangeValueClamped(
		FVector2D(DistanceMin, DistanceMax),
		FVector2D(ScaleNear, ScaleFar),
		Dist
	);

	int32 SizeX, SizeY;
	GetViewportSize(SizeX, SizeY);

	bool bInside =
		ScreenPos.X > -50 && ScreenPos.X < SizeX + 50 &&
		ScreenPos.Y > -50 && ScreenPos.Y < SizeY + 50;

	StaminaWidget->SetTrackingPosition(ScreenPos, Scale, bInside);
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

void AUK_PlayerController::ConnectStaminaWidget()
{
	if ( !StaminaWidget ) return;

	APawn* MyPawn = GetPawn();
	if ( !MyPawn ) return;

	UStatusComponent* StatusComp =
		MyPawn->FindComponentByClass<UStatusComponent>();

	if ( !StatusComp ) return;

	StaminaWidget->BindStatusComponent(StatusComp);
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