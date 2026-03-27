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
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "Systems/UK_GameInstance.h"
#include "UI/Inventory/UK_InvMain.h"
#include <UI/InGame/CreftWeaponUI/UK_Crafting.h>

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
			MainHUD->AddToViewport(0);
			MainHUD->SetVisibility(ESlateVisibility::Collapsed);

			if ( UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance()) )
			{
				MainHUD->RefreshAllStatus();
			}
		}
	}

	if ( StaminaWidgetClass )
	{
		StaminaWidget = CreateWidget<UUK_Stamina>(this, StaminaWidgetClass);
		if ( StaminaWidget )
		{
			StaminaWidget->AddToViewport(0);
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
				}, 2.0f, false);
		}
	}
}

void AUK_PlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if ( !IsLocalController() ) return;

	ApplyInputState(EInputState::Game);
}

void AUK_PlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);

	if ( !IsLocalController() ) return;

	ConnectStaminaWidget();

	AUK_CharacterBase* MyCharacter = Cast<AUK_CharacterBase>(pawn);
	if ( !MyCharacter ) return;

	UUK_InputConfig* InputConfig_Player = MyCharacter->InputMappingConfig;
	if ( !InputConfig_Player ) return;

	IMC = InputConfig_Player->GetIMC();
	if ( !IMC ) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if ( !LocalPlayer ) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if ( !Subsystem ) return;

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(IMC, 0);

	UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings();
	if ( Settings )
	{
		if ( !Settings->IsMappingContextRegistered(this->IMC) )
		{
			Settings->RegisterInputMappingContext(this->IMC);
		}
	}

	MyCharacter->OnDead.RemoveAll(this);
	MyCharacter->OnDead.AddDynamic(this, &ThisClass::ShowGameOverUI);

	SetViewTargetWithBlend(pawn);
}

// ===== UI 생성 =====

void AUK_PlayerController::Client_CreatePlayerUI_Implementation()
{
	if ( !IsLocalController() ) return;

	if ( SettingWidgetClass )
	{
		SettingWidget = CreateWidget<UUK_Setting>(this, SettingWidgetClass);
		if ( SettingWidget )
		{
			SettingWidget->AddToViewport(10); // 다른 UI보다 위에
			SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

// ===== 입력 상태 관리 =====

bool AUK_PlayerController::InputKey_Check(const FInputKeyParams& Params)
{
	if ( Params.Key == EKeys::Escape && Params.Event == IE_Pressed )
	{
		CloseOpenWidget();
		return true;
	}
	return Super::InputKey(Params);
}

void AUK_PlayerController::ApplyInputState(EInputState NewState)
{
	CurrentInputState = NewState;

	switch ( CurrentInputState )
	{
	case EInputState::Game:
	{
		SetAllGameUIInputVisibility(true);
		SetIgnoreLookInput(false);
		SetIgnoreMoveInput(false);

		FInputModeGameOnly Mode;
		Mode.SetConsumeCaptureMouseDown(false);
		SetInputMode(Mode);
		SetCursorVisible(false);

		FSlateApplication::Get().SetAllUserFocusToGameViewport();
		break;
	}
	case EInputState::UI:
	{
		SetAllGameUIInputVisibility(false);
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);

		FInputModeUIOnly Mode;
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

void AUK_PlayerController::SetCursorVisible(bool bVisible)
{
	bShowMouseCursor = bVisible;
	bEnableClickEvents = bVisible;
	bEnableMouseOverEvents = bVisible;
}

void AUK_PlayerController::ToggleMouseCursor()
{
	bMouseCursorEnabled = !bMouseCursorEnabled;
	SetCursorVisible(bMouseCursorEnabled);
	SetIgnoreLookInput(bMouseCursorEnabled);
}

void AUK_PlayerController::SetAllGameUIInputVisibility(bool bVisible)
{
	ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if ( MainHUD ) MainHUD->SetVisibility(NewVisibility);
	if ( StaminaWidget ) StaminaWidget->SetVisibility(NewVisibility);
}

// ===== 다른 UI 모두 닫기 (Setting 제외) =====

void AUK_PlayerController::CloseAllExceptSetting()
{
	if ( InventoryWidget && InventoryWidget->IsInViewport() )
		CloseInventoryUI();

	if ( WeaponCraftingWidget && WeaponCraftingWidget->IsInViewport() )
		CloseWeaponCraftingUI();

	if ( QuestWidget && QuestWidget->IsInViewport() )
		HideQuestUI();
}

// ===== Setting UI =====

void AUK_PlayerController::Setting_UI()
{
	if ( !SettingWidget ) return;

	// 다른 UI 먼저 닫기
	CloseAllExceptSetting();

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

void AUK_PlayerController::CloseSettingUI()
{
	if ( !SettingWidget ) return;

	SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
	bIsSetting = false;

	ApplyInputState(EInputState::Game);
	GetWorldTimerManager().UnPauseTimer(StaminaTrackingTimer);
}

void AUK_PlayerController::OpenSettingAndCloseOtherUI()
{
	// 세팅이 이미 열려있으면 닫기
	if ( SettingWidget &&
		SettingWidget->IsInViewport() &&
		SettingWidget->GetVisibility() != ESlateVisibility::Collapsed )
	{
		bIsSetting = true;
		Setting_UI();
		return;
	}

	// 세팅 열기 (Setting_UI 내부에서 다른 UI 닫음)
	if ( SettingWidget )
	{
		bIsSetting = false;
		Setting_UI();
	}
}

bool AUK_PlayerController::CloseOpenWidget()
{
	if ( SettingWidget && SettingWidget->GetVisibility() != ESlateVisibility::Collapsed )
	{
		CloseSettingUI();
		return true;
	}
	/*if ( SettingWidget &&
		SettingWidget->IsInViewport() &&
		SettingWidget->GetVisibility() != ESlateVisibility::Collapsed )
	{
		SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
		bIsSetting = false;
		ApplyInputState(EInputState::Game);
		GetWorldTimerManager().UnPauseTimer(StaminaTrackingTimer);
		return true;
	}*/
	if ( InventoryWidget && InventoryWidget->IsInViewport() )
	{
		CloseInventoryUI();
		return true;
	}
	if ( WeaponCraftingWidget && WeaponCraftingWidget->IsInViewport() )
	{
		CloseWeaponCraftingUI();
		return true;
	}
	if ( QuestWidget && QuestWidget->IsInViewport() )
	{
		HideQuestUI();
		return true;
	}

	return false;
}

// ===== Inventory UI =====

void AUK_PlayerController::Inventory_UI()
{
	if ( !IsLocalController() ) return;
	if ( !InventoryWidgetClass ) return;

	// 이미 열려있으면 닫기
	if ( InventoryWidget && InventoryWidget->IsInViewport() )
	{
		CloseInventoryUI();
		return;
	}

	// 다른 UI 닫기
	if ( SettingWidget && SettingWidget->GetVisibility() != ESlateVisibility::Collapsed )
	{
		CloseSettingUI();
	}

	if ( WeaponCraftingWidget && WeaponCraftingWidget->IsInViewport() )
		CloseWeaponCraftingUI();
	if ( QuestWidget && QuestWidget->IsInViewport() )
		HideQuestUI();

	InventoryWidget = CreateWidget<UUK_InvMain>(this, InventoryWidgetClass);
	if ( !InventoryWidget ) return;

	InventoryWidget->AddToViewport(0);
	ApplyInputState(EInputState::UI);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void AUK_PlayerController::CloseInventoryUI()
{
	if ( !InventoryWidget ) return;

	InventoryWidget->RemoveFromParent();
	InventoryWidget = nullptr;

	ApplyInputState(EInputState::Game);
}

// ===== WeaponCrafting UI =====

void AUK_PlayerController::WeaponCrafting_UI()
{
	if ( !IsLocalController() ) return;
	if ( !WeaponCraftingWidgetClass ) return;

	// 이미 열려있으면 닫기
	if ( WeaponCraftingWidget && WeaponCraftingWidget->IsInViewport() )
	{
		CloseWeaponCraftingUI();
		return;
	}

	// 다른 UI 닫기
	if ( SettingWidget && SettingWidget->GetVisibility() != ESlateVisibility::Collapsed )
	{
		CloseSettingUI();
	}
	if ( InventoryWidget && InventoryWidget->IsInViewport() )
		CloseInventoryUI();
	if ( QuestWidget && QuestWidget->IsInViewport() )
		HideQuestUI();

	WeaponCraftingWidget = CreateWidget<UUK_Crafting>(this, WeaponCraftingWidgetClass);
	if ( !WeaponCraftingWidget ) return;

	WeaponCraftingWidget->AddToViewport(0);
	ApplyInputState(EInputState::UI);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(WeaponCraftingWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void AUK_PlayerController::CloseWeaponCraftingUI()
{
	if ( !WeaponCraftingWidget ) return;

	WeaponCraftingWidget->RemoveFromParent();
	WeaponCraftingWidget = nullptr;

	ApplyInputState(EInputState::Game);
}

// ===== Quest UI =====

void AUK_PlayerController::ShowQuestUI(const FName& QuestID, const FText& NPCName, const FText& Dialogue, const FText& QuestDesc)
{
	if ( !IsLocalController() ) return;
	if ( QuestWidget ) return;
	if ( !QuestWidgetClass ) return;

	QuestWidget = CreateWidget<UUK_Quest>(this, QuestWidgetClass);
	if ( !QuestWidget ) return;

	QuestWidget->AddToViewport(0);

	FText CurrentSpeakerName = NPCName;
	FText CurrentDialogueText = Dialogue;
	FText CurrentChoiceText = FText::GetEmpty();
	bool bUseLiveDialogue = false;

	UUKQuestUIManagerSubsystem* QuestUIManager = GetGameInstance()->GetSubsystem<UUKQuestUIManagerSubsystem>();
	if ( QuestUIManager )
	{
		const FUKCurrentDialogueUIData UIData = QuestUIManager->GetCurrentDialogueUIData();
		if ( !UIData.DialogueId.IsNone() )
		{
			bUseLiveDialogue = true;
			if ( !UIData.SpeakerName.IsEmpty() ) CurrentSpeakerName = UIData.SpeakerName;
			if ( !UIData.DialogueText.IsEmpty() ) CurrentDialogueText = UIData.DialogueText;
			if ( UIData.Choices.Num() > 0 ) CurrentChoiceText = UIData.Choices[ 0 ].ChoiceText;
		}
	}

	QuestWidget->SetQuestUI(QuestID, CurrentSpeakerName, CurrentDialogueText, QuestDesc, CurrentChoiceText, FText::FromString(TEXT("닫기")));

	if ( bUseLiveDialogue )
		QuestWidget->RefreshDialogueUI();

	ApplyInputState(EInputState::UI);
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(QuestWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void AUK_PlayerController::HideQuestUI()
{
	if ( !IsLocalController() ) return;
	if ( !QuestWidget ) return;

	QuestWidget->RemoveFromParent();
	QuestWidget = nullptr;

	ApplyInputState(EInputState::Game);
}

// ===== 기타 =====

void AUK_PlayerController::ClearAllWidgets()
{
	if ( MainHUD && MainHUD->IsInViewport() )
	{
		MainHUD->SetVisibility(ESlateVisibility::Collapsed);
		MainHUD = nullptr;
	}
	if ( StaminaWidget && StaminaWidget->IsInViewport() )
	{
		StaminaWidget->SetVisibility(ESlateVisibility::Collapsed);
		StaminaWidget = nullptr;
	}
	if ( SettingWidget && SettingWidget->IsInViewport() )
	{
		SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
		SettingWidget = nullptr;
	}
	if ( QuestWidget && QuestWidget->IsInViewport() )
	{
		QuestWidget->SetVisibility(ESlateVisibility::Collapsed);
		QuestWidget = nullptr;
	}
	if ( ShopWidget && ShopWidget->IsInViewport() )
	{
		ShopWidget->SetVisibility(ESlateVisibility::Collapsed);
		ShopWidget = nullptr;
	}
	//if ( GameOverWidget && GameOverWidget->IsInViewport() )
	//{
	//	GameOverWidget->RemoveFromParent();
	//	GameOverWidget = nullptr;
	//}
}

void AUK_PlayerController::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	float CurrentHealth = Data.NewValue;
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %f"), CurrentHealth);

	if ( CurrentHealth <= 190.f )
		ShowGameOverUI();
}

void AUK_PlayerController::ShowGameOverUI()
{

	if ( WeaponCraftingWidget && WeaponCraftingWidget->IsInViewport() )
	{
		CloseWeaponCraftingUI();
		return;
	}

	// 다른 UI 닫기
	if ( SettingWidget && SettingWidget->GetVisibility() != ESlateVisibility::Collapsed )
	{
		CloseSettingUI();
	}
	if ( InventoryWidget && InventoryWidget->IsInViewport() )
		CloseInventoryUI();
	if ( QuestWidget && QuestWidget->IsInViewport() )
		HideQuestUI();

	if ( !GameOverWidgetClass ) return;

	if ( !GameOverWidget )
		GameOverWidget = CreateWidget<UUK_GameOver>(this, GameOverWidgetClass);

	if ( GameOverWidget && !GameOverWidget->IsInViewport() )
	{
		GameOverWidget->AddToViewport(0);

		ApplyInputState(EInputState::UI);
	}
}

void AUK_PlayerController::HideGameOverUI()
{
	if ( !GameOverWidget ) return;

	// 화면에서 제거
	GameOverWidget->RemoveFromParent();
	GameOverWidget = nullptr;

	// 입력 상태를 다시 게임으로 복구
	ApplyInputState(EInputState::Game);
}

void AUK_PlayerController::ShowShopUI(TSubclassOf<UUserWidget> ShopWidgetClass)
{
	if ( !ShopWidgetClass ) return;

	ShopWidget = CreateWidget<UUserWidget>(this, ShopWidgetClass);
	if ( ShopWidget )
	{
		ShopWidget->AddToViewport(0);
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ShopWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void AUK_PlayerController::HideShopUI()
{
	if ( ShopWidget )
	{
		ShopWidget->RemoveFromParent();
		ShopWidget = nullptr;
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

void AUK_PlayerController::ConnectStaminaWidget()
{
	if ( !StaminaWidget ) return;

	APawn* MyPawn = GetPawn();
	if ( !MyPawn ) return;
}

void AUK_PlayerController::UpdateStaminaTracking()
{
	if ( !IsLocalController() ) return;
	if ( !StaminaWidget ) return;
	if ( !PlayerCameraManager ) return;

	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	if ( GI && GI->PersistentLoadingWidget )
	{
		StaminaWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

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