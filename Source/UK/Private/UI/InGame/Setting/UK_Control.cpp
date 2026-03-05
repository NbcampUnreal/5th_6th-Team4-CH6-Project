// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/Setting/UK_Control.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputActionValue.h"
//#include "EnhancedPlayerMappableKeyProfile.h"

void UUK_Control::NativeConstruct()
{
	Super::NativeConstruct();
	if ( ControlBackButton )
		ControlBackButton->OnClicked.AddDynamic(this, &UUK_Control::OnBackButtonClicked);
}

void UUK_Control::OnBackButtonClicked()
{
	if ( ParentSettingWidget )
	{
		ParentSettingWidget->SetVisibility(ESlateVisibility::Visible);
		RemoveFromParent();
	}
}

void UUK_Control::StartRebind(FName MappingName)
{
	PendingMapping = MappingName;
	bListeningForKey = true;

	// 여기서 UI를 기다리는 중이라는 표시를 띄워줄 수 있음
}

FReply UUK_Control::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if ( bListeningForKey )
	{
		FKey NewKey = InKeyEvent.GetKey();
		bListeningForKey = false;

		ApplyRebind(PendingMapping, NewKey);

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UUK_Control::ApplyRebind(FName MappingName, FKey NewKey)
{
	APlayerController* PC = GetOwningPlayer();
	if ( !PC ) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if ( !LP ) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if ( !Subsystem ) return;

	UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings();
	if ( !Settings ) return;

	UEnhancedPlayerMappableKeyProfile* Profile =
		Settings->GetCurrentKeyProfile();
	if ( !Profile ) return;

	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;
	Args.NewKey = NewKey;
	FGameplayTagContainer FailureReason;
	Settings->MapPlayerKey(Args, FailureReason);

	Settings->ApplySettings();
	Settings->SaveSettings();
}