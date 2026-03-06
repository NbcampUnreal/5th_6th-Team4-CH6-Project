// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/Setting/UK_Sound.h"

void UUK_Sound::NativeConstruct()
{
	Super::NativeConstruct();
	if ( SoundBackButton )
		SoundBackButton->OnClicked.AddDynamic(this, &UUK_Sound::OnBackButtonClicked);
}

void UUK_Sound::OnBackButtonClicked()
{
	if ( ParentSettingWidget )
	{
		ParentSettingWidget->SetVisibility(ESlateVisibility::Visible);
		RemoveFromParent();
	}
}