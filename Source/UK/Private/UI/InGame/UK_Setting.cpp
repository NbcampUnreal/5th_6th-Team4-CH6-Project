// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_Setting.h"
#include "UI/InGame/Setting/UK_Sound.h"
#include "UI/InGame/Setting/UK_Screen.h"
#include "UI/InGame/Setting/UK_Control.h"
#include "Character/UK_PlayerController.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/UserWidget.h"

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

//void UUK_Setting::ClearAllWidgets()
//{
//	if ( SoundWidget && SoundWidget->IsInViewport() )
//	{
//		SoundWidget->RemoveFromParent();
//		SoundWidget = nullptr;
//	}
//
//	// 2. 비디오(스크린) 창 제거
//	if ( ControlWidget && ControlWidget->IsInViewport() )
//	{
//		ControlWidget->RemoveFromParent();
//		ControlWidget = nullptr;
//	}
//
//	// 3. 컨트롤 창 제거
//	if ( VideoWidget && VideoWidget->IsInViewport() )
//	{
//		VideoWidget->RemoveFromParent();
//		VideoWidget = nullptr;
//	}
//}

void UUK_Setting::ClearAllWidgets()
{
	// Switcher를 쓰면 RemoveFromParent를 할 필요가 없습니다.
	// 필요하다면 기본 화면(인덱스 0번 등)으로 돌리는 로직을 넣으세요.
}

void UUK_Setting::OnSoundButtonClicked()
{
	SetInputConfig();
}

void UUK_Setting::OnVideoButtonClicked()
{
	SetInputConfig();
}

void UUK_Setting::OnControlButtonClicked()
{
	SetInputConfig();
}

void UUK_Setting::SetInputConfig()
{
	if ( !UK_PC ) return;

	FInputModeGameAndUI InputMode;

	InputMode.SetWidgetToFocus(SettingSwitcher->GetActiveWidget()->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	InputMode.SetHideCursorDuringCapture(false);

	UK_PC->SetInputMode(InputMode);
	UK_PC->bShowMouseCursor = true;
}

void UUK_Setting::OnExitButtonClicked()
{
	if ( UK_PC )
	{
		UK_PC->Setting_UI(); // 토글 기능이 있다면 호출
	}
	else
	{
		RemoveFromParent();
	}
}