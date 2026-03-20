// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController.h"
#include "Character/UK_CharacterBase.h"
#include "UI/InGame/UK_MainHUD.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "DataAsset/UK_InputConfig.h"
#include "UI/InGame/UK_Quest.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UI/InGame/UK_GameOver.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Systems/UK_GameInstance.h"


AUK_PlayerController::AUK_PlayerController()
	: bMouseCursorEnabled(false)
{
	QuestWidget = nullptr;
	StaminaWidget = nullptr;
	SettingWidget = nullptr;
	MainHUD = nullptr;
	GameOverWidget = nullptr;
	ShopWidget = nullptr;
	bIsSetting = false;
	CurrentInputState = EInputState::Game;
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

	if ( MainHUDClass )
	{
		MainHUD = CreateWidget<UUK_MainHUD>(this, MainHUDClass);
		if ( MainHUD )
		{
			MainHUD->AddToViewport();
			MainHUD->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if ( StaminaWidgetClass )
	{
		StaminaWidget = CreateWidget<UUK_Stamina>(this, StaminaWidgetClass);
		if ( StaminaWidget )
		{
			StaminaWidget->AddToViewport();
			StaminaWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if ( UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance()) )
	{
		if ( GI->PersistentLoadingWidget )
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, [ GI ] ()
				{
					GI->PersistentLoadingWidget->TargetValue = 1.0f;
				}, 0.3f, false);
		}
	}
	//FOnAttributeChangeData Data;
	//// 		//UpdateStaminaBar(StatusPtr->CurrentStamina, StatusPtr->MaxStamina); 
	//// HP 초기값 세팅 및 바인딩
	//Data.NewValue = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute());
	//OnHealthChanged(Data);
	//Data.NewValue = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetHealthAttribute());
	//OnHealthChanged(Data);

	//ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute()).
	//	AddUObject(this, &AUK_PlayerController::OnHealthChanged);
	//ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetHealthAttribute()).
	//	AddUObject(this, &AUK_PlayerController::OnHealthChanged);

	//FOnAttributeChangeData Data;
	//if ( ASC )
	//{
	//	// 현재 체력 값 가져와서 초기 체크

	//	/*Data.NewValue = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetHealthAttribute());
	//	OnHealthChanged(Data);

	//	ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetHealthAttribute()).
	//		AddUObject(this, &AUK_PlayerController::OnHealthChanged);*/

	//}

	//if ( UUK_PlayerStatusAttributeSet* AttributeSet = ASC->GetSet<UUK_PlayerStatusAttributeSet>() )
	//{
	//	// Health 변화 바인딩
	//	ASC->GetGameplayAttributeValueChangeDelegate(
	//		UUK_PlayerStatusAttributeSet::GetHealthAttribute()
	//	).AddUObject(this, &AUK_PlayerController::OnHealthChanged);
	//}
}

void AUK_PlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if ( !IsLocalController() ) return;
	//ApplyInputState(EInputState::Game);
	SetCursorVisible(false);
}

void AUK_PlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
	if ( !IsLocalController() || !pawn ) return;

	AUK_CharacterBase* MyCharacter = Cast<AUK_CharacterBase>(pawn);

	if ( !MyCharacter ) return;
	//if ( !StaminaWidget && StaminaWidgetClass )
	//{
	//	StaminaWidget = CreateWidget<UUK_Stamina>(this, StaminaWidgetClass);
	//	if ( StaminaWidget )
	//	{
	//		StaminaWidget->AddToViewport(1);
	//		StaminaWidget->SetVisibility(ESlateVisibility::Visible);
	//	}
	//}

	ConnectStaminaWidget();

	if ( StaminaWidget )
		UpdateStaminaTracking();
	// 입력 설정
	if ( UUK_InputConfig* InputConfig = MyCharacter->InputMappingConfig )
	{
		IMC = InputConfig->GetIMC();
		if ( IMC )
		{
			if ( ULocalPlayer* LocalPlayer = GetLocalPlayer() )
			{
				UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
				if ( Subsystem )
				{
					Subsystem->ClearAllMappings();
					Subsystem->AddMappingContext(IMC, 0);

					if ( UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings() )
					{
						if ( !Settings->IsMappingContextRegistered(IMC) )
							Settings->RegisterInputMappingContext(IMC);
					}
				}
			}
		}
	}

	MyCharacter->OnDead.RemoveAll(this);
	MyCharacter->OnDead.AddDynamic(this, &ThisClass::ShowGameOverUI);

}

void AUK_PlayerController::ClearAllWidgets()
{
	// 메인 HUD
	if ( MainHUD && MainHUD->IsInViewport() )
	{
		MainHUD->RemoveFromParent();
		MainHUD = nullptr;
	}

	//// 스태미나
	if ( StaminaWidget && StaminaWidget->IsInViewport() )
	{
		StaminaWidget->RemoveFromParent();
		StaminaWidget = nullptr;
	}

	// 세팅
	if ( SettingWidget && SettingWidget->IsInViewport() )
	{
		SettingWidget->RemoveFromParent();
		SettingWidget = nullptr;
	}

	// 퀘스트
	if ( QuestWidget && QuestWidget->IsInViewport() )
	{
		QuestWidget->RemoveFromParent();
		QuestWidget = nullptr;
	}

	// 상점
	if ( ShopWidget && ShopWidget->IsInViewport() )
	{
		ShopWidget->RemoveFromParent();
		ShopWidget = nullptr;
	}

	// 게임오버
	if ( GameOverWidget && GameOverWidget->IsInViewport() )
	{
		GameOverWidget->RemoveFromParent();
		GameOverWidget = nullptr;
	}
}

template <typename T>
T* AUK_PlayerController::ShowOnlyWidget(TSubclassOf<T> WidgetClass, int32 ZOrder)
{
	if ( !WidgetClass ) return nullptr;

	ClearAllWidgets();

	T* NewWidget = CreateWidget<T>(this, WidgetClass);
	if ( NewWidget )
	{
		NewWidget->AddToViewport(ZOrder);
	}

	return NewWidget;
}

// -----  UI 생성 -----

void AUK_PlayerController::Client_CreatePlayerUI_Implementation()
{
	if ( !IsLocalController() ) return;

	if ( SettingWidgetClass && !SettingWidget )
	{
		// ShowOnlyWidget을 쓰면 HUD 포인터가 날아갑니다. 직접 생성하세요.
		SettingWidget = CreateWidget<UUK_Setting>(this, SettingWidgetClass);
		if ( SettingWidget )
		{
			SettingWidget->AddToViewport(99);
			SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

// ----- 입력 상태 관리 -----

void AUK_PlayerController::ApplyInputState(EInputState NewState)
{
	CurrentInputState = NewState;

	SetAllGameUIInputVisibility(false);

	switch ( CurrentInputState )
	{
	case EInputState::Game:
	{
		SetIgnoreLookInput(false);
		SetIgnoreMoveInput(false);

		FInputModeGameOnly Mode;
		SetInputMode(Mode);
		SetCursorVisible(false);

		if ( MainHUD )
		{
			MainHUD->SetVisibility(ESlateVisibility::Visible);
		}
		if ( StaminaWidget )
		{
			StaminaWidget->SetVisibility(ESlateVisibility::Visible);
		}

		break;
	}

	case EInputState::UI:
	{
		SetAllGameUIInputVisibility(false);
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);

		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		SetInputMode(Mode);
		SetCursorVisible(true);
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

void AUK_PlayerController::SetAllGameUIInputVisibility(bool bVisible)
{
	ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	// 개별 위젯들 가시성 조절
	if ( MainHUD ) MainHUD->SetVisibility(NewVisibility);
	if ( StaminaWidget ) StaminaWidget->SetVisibility(NewVisibility);

	// 퀘스트나 상점 위젯이 떠 있다면 그것들도 숨김/제거
	/*if ( QuestWidget ) QuestWidget->SetVisibility(NewVisibility);
	if ( ShopWidget ) ShopWidget->SetVisibility(NewVisibility);*/
}

void AUK_PlayerController::Setting_UI()
{
	if ( !SettingWidget ) return;

	bIsSetting = !bIsSetting;

	if ( bIsSetting )
	{
		ApplyInputState(EInputState::UI);

		SettingWidget->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(SettingWidget->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);

		GetWorldTimerManager().PauseTimer(StaminaTrackingTimer);
	}
	else
	{
		SettingWidget->SetVisibility(ESlateVisibility::Collapsed);

		ApplyInputState(EInputState::Game);

		GetWorldTimerManager().UnPauseTimer(StaminaTrackingTimer);
	}
}

void AUK_PlayerController::UpdateStaminaTracking()
{
	if ( !IsLocalController() ) return;
	if ( !StaminaWidget ) return;
	if ( !PlayerCameraManager ) return;


	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	if ( GI && GI->PersistentLoadingWidget )
	{
		// 로딩 중이면 트래킹 계산을 하지 않고 위젯을 숨깁니다.
		StaminaWidget->SetVisibility(ESlateVisibility::Collapsed);
		return; // 여기서 함수 종료! 아래의 SetTrackingPosition을 호출하지 않음.
	}

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

	// UStatusComponent* StatusComp =
	// 	MyPawn->FindComponentByClass<UStatusComponent>();
	//
	// if ( StatusComp )
	// {
	// 	StaminaWidget->BindStatusComponent(StatusComp);
	// }
}

// -------- 퀘스트 UI Interaction (무현 구현중)
 
void AUK_PlayerController::ShowQuestUI(const FName& QuestID, const FText& NPCName, const FText& Dialogue, const FText& QuestDesc)
{
	UE_LOG(LogTemp, Warning, TEXT("[PC] ShowQuestUI 호출됨"));

	if ( !IsLocalController() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] LocalController 아님"));
		return;
	}

	if ( QuestWidget )
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] QuestWidget 이미 있음"));
		return;
	}

	if ( !QuestWidgetClass )
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] QuestWidgetClass 없음"));
		return;
	}


	QuestWidget = ShowOnlyWidget<UUK_Quest>(QuestWidgetClass, 0);
	if ( !QuestWidget ) return;

	//QuestWidget->AddToViewport();
	//QuestWidget = CreateWidget<UUK_Quest>(this, QuestWidgetClass);
	if ( !QuestWidget )
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] CreateWidget 실패"));
		return;
	}

	//QuestWidget->AddToViewport();
	UE_LOG(LogTemp, Warning, TEXT("[PC] AddToViewport 성공"));


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

void AUK_PlayerController::HideQuestUI()
{
	if ( !IsLocalController() ) return;

	if ( !QuestWidget ) return;

	QuestWidget->RemoveFromParent();
	QuestWidget = nullptr;

	ApplyInputState(EInputState::Game);
	SetCursorVisible(false);
}

void AUK_PlayerController::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	float CurrentHealth = Data.NewValue;
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %f"), CurrentHealth);

	if ( CurrentHealth <= 190.f )
	{
		ShowGameOverUI();
	}
}

void AUK_PlayerController::ShowGameOverUI()
{
	if ( GameOverWidget && GameOverWidget->IsInViewport() ) return;
	if ( !GameOverWidgetClass ) return;

	ClearAllWidgets();

	GameOverWidget = CreateWidget<UUK_GameOver>(this, GameOverWidgetClass);
	if ( GameOverWidget )
	{
		GameOverWidget->AddToViewport(100);
	}
}

void AUK_PlayerController::ShowShopUI(TSubclassOf<UUserWidget> ShopWidgetClass)
{
	if (!ShopWidgetClass) return;
	
	ShopWidget = ShowOnlyWidget<UUserWidget>(ShopWidgetClass, 0);
	if (ShopWidget)
	{
		//ShopWidget->AddToViewport();
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ShopWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void AUK_PlayerController::HideShopUI()
{
	if (ShopWidget)
	{
		ShopWidget->RemoveFromParent();
		ShopWidget = nullptr;
		
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

