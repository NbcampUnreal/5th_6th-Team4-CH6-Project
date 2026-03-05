// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController.h"
#include "ActorComponent/UK_InputComponent.h"
#include "Character/UK_CharacterBase.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "UI/InGame/UK_Quest.h"


AUK_PlayerController::AUK_PlayerController()
	: bMouseCursorEnabled(false)
{
	QuestWidget = nullptr;
}

void AUK_PlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if ( PlayerCameraManager )
	{
		PlayerCameraManager->ViewPitchMax = 45.f;
		PlayerCameraManager->ViewPitchMin = -60.f;
	}
}

void AUK_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if ( !IsLocalController() ) return;

	Client_CreatePlayerUI();

	// 게임 입력 상태
	ApplyInputState(EInputState::Game);

	GetWorldTimerManager().SetTimer(
		StaminaTrackingTimer,
		this,
		&AUK_PlayerController::UpdateStaminaTracking,
		0.005f,
		true
	);
}

void AUK_PlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if ( !IsLocalController() ) return;

	// 맵 이동 후 게임 상태
	ApplyInputState(EInputState::Game);
	SetCursorVisible(false);
}

void AUK_PlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);

	if ( !IsLocalController() )
		return;

	if ( IsLocalController() )
	{
		ConnectStaminaWidget();
	}


	AUK_CharacterBase* MyCharacter = Cast<AUK_CharacterBase>(pawn);
	if ( !MyCharacter ) return;

	UUK_InputConfig* InputConfig = MyCharacter->InputMappingConfig;
	if ( !InputConfig ) return;

	UInputMappingContext* IMC = InputConfig->GetIMC();
	if ( !IMC ) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if ( !LocalPlayer ) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if ( !Subsystem ) return;

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(IMC, 0);

	UEnhancedInputUserSettings* Settings =
		Subsystem->GetUserSettings();

	if ( Settings->IsMappingContextRegistered(IMC) )
	{

	}
	else
	{
		Settings->RegisterInputMappingContext(IMC);
	}


	//UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	//if ( Subsystem )
	//{
	//	Subsystem->AddMappingContext(IMC, 0);

}

// -----  UI 생성 -----

void AUK_PlayerController::Client_CreatePlayerUI_Implementation()
{
	if ( !IsLocalController() ) return;

	// ----- Stamina UI -----
	if ( StaminaWidgetClass )
	{
		StaminaWidget = CreateWidget<UUK_Stamina>(this, StaminaWidgetClass);

		if ( StaminaWidget )
		{
			StaminaWidget->AddToViewport();
			StaminaWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// ----- Setting UI -----
	if ( SettingWidgetClass )
	{
		SettingWidget = CreateWidget<UUK_Setting>(this, SettingWidgetClass);

		if ( SettingWidget )
		{
			SettingWidget->AddToViewport();
			SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

// ----- 입력 상태 관리 -----

void AUK_PlayerController::ApplyInputState(EInputState NewState)
{
	CurrentInputState = NewState;

	switch ( CurrentInputState )
	{
	case EInputState::Game:
	{
		SetIgnoreLookInput(false);
		SetIgnoreMoveInput(false);

		FInputModeGameOnly Mode;
		SetInputMode(Mode);
		break;
	}

	case EInputState::UI:
	{
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);

		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
		break;
	}

	case EInputState::Cutscene:
	{
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);

		FInputModeGameOnly Mode;
		SetInputMode(Mode);
		break;
	}
	}
}

// 커서 표시만 담당 (입력 상태랑 분리)
void AUK_PlayerController::SetCursorVisible(bool bVisible)
{
	bShowMouseCursor = bVisible;

	bEnableClickEvents = bVisible;
	bEnableMouseOverEvents = bVisible;
}

//  게임 중 커서 토글 (카메라 안 멈춤)
void AUK_PlayerController::ToggleMouseCursor()
{
	bMouseCursorEnabled = !bMouseCursorEnabled;

	SetCursorVisible(bMouseCursorEnabled);

	// 캐릭터 움직임은 가능하게 + 화면 회전은 불가
	SetIgnoreLookInput(bMouseCursorEnabled);
}

//  Setting UI (카메라 멈춤 모드)

void AUK_PlayerController::Setting_UI()
{
	if ( !SettingWidget ) return;

	bIsSetting = !bIsSetting;

	if ( bIsSetting )
	{
		// ----- 열기 -----
		SettingWidget->SetVisibility(ESlateVisibility::Visible);

		ApplyInputState(EInputState::UI);

		// 캐릭터 움직임 + 화면 회전은 불가
		SetCursorVisible(true);
		SetIgnoreLookInput(true);
		GetWorldTimerManager().PauseTimer(StaminaTrackingTimer);
	}
	else
	{
		// ----- 닫기 -----
		SettingWidget->SetVisibility(ESlateVisibility::Collapsed);

		ApplyInputState(EInputState::Game);
		SetCursorVisible(false);
		SetIgnoreLookInput(false);
		GetWorldTimerManager().UnPauseTimer(StaminaTrackingTimer);
	}
}

void AUK_PlayerController::UpdateStaminaTracking()
{
	if ( !IsLocalController() ) return;
	if ( !StaminaWidget ) return;
	if ( !PlayerCameraManager ) return;

	APawn* MyPawn = GetPawn();
	if ( !MyPawn )
	{
		StaminaWidget->SetTrackingPosition(FVector2D::ZeroVector, 1.f, false);
		return;
	}

	// 카메라 정보
	FVector CamLoc = PlayerCameraManager->GetCameraLocation();
	FRotator CamRot = PlayerCameraManager->GetCameraRotation();

	FVector CamRight = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
	FVector CamForward = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);

	// 스태미나 UI가 붙을 월드 위치 계산
	FVector BaseLocation = MyPawn->GetActorLocation() + Stemina_Location;
	FVector TargetWorldPos = BaseLocation + ( CamRight * SideDistance );

	// 월드를 스크린 좌표 변환
	FVector2D ScreenPos;
	bool bProjected = ProjectWorldLocationToScreen(TargetWorldPos, ScreenPos, true);

	if ( !bProjected )
	{
		StaminaWidget->SetTrackingPosition(ScreenPos, 1.f, false);
		return;
	}

	// 카메라 뒤쪽에 있으면 숨김
	FVector ToTarget = ( TargetWorldPos - CamLoc ).GetSafeNormal();
	float Dot = FVector::DotProduct(CamForward, ToTarget);

	if ( Dot < 0.15f )
	{
		StaminaWidget->SetTrackingPosition(ScreenPos, 1.f, false);
		return;
	}

	// 거리 기반 스케일 계산
	float Dist = FVector::Dist(CamLoc, TargetWorldPos);

	float Scale = FMath::GetMappedRangeValueClamped(
		FVector2D(DistanceMin, DistanceMax),
		FVector2D(ScaleNear, ScaleFar),
		Dist
	);

	// 화면 안에 있는지 체크
	int32 SizeX, SizeY;
	GetViewportSize(SizeX, SizeY);

	bool bInside =
		ScreenPos.X > -50 && ScreenPos.X < SizeX + 50 &&
		ScreenPos.Y > -50 && ScreenPos.Y < SizeY + 50;

	StaminaWidget->SetTrackingPosition(ScreenPos, Scale, bInside);
}

//  ----- 스태미나 -----

void AUK_PlayerController::ConnectStaminaWidget()
{
	if ( !StaminaWidget ) return;

	APawn* MyPawn = GetPawn();
	if ( !MyPawn ) return;

	UStatusComponent* StatusComp =
		MyPawn->FindComponentByClass<UStatusComponent>();

	if ( StatusComp )
	{
		StaminaWidget->BindStatusComponent(StatusComp);
	}
}

// -------- 퀘스트 UI Interaction (무현 구현중)
 
void AUK_PlayerController::Client_ShowQuestUI_Implementation(const FName& QuestID,const FText& NPCName,const FText& Dialogue,const FText& QuestDesc)
{
	if ( !IsLocalController() ) return;

	if ( QuestWidget ) return;
	if ( !QuestWidgetClass ) return;

	QuestWidget = CreateWidget<UUK_Quest>(this, QuestWidgetClass);
	if ( !QuestWidget ) return;

	QuestWidget->AddToViewport();

	QuestWidget->SetQuestUI(
		QuestID,
		NPCName,
		Dialogue,
		QuestDesc,
		FText::FromString(TEXT("수락")),
		FText::FromString(TEXT("닫기"))
	);

	ApplyInputState(EInputState::UI);
	SetCursorVisible(true);
}

void AUK_PlayerController::Client_HideQuestUI_Implementation()
{
	if (!IsLocalController()) return;

	if (!QuestWidget) return;

	QuestWidget->RemoveFromParent();
	QuestWidget = nullptr;

	ApplyInputState(EInputState::Game);
	SetCursorVisible(false);
}