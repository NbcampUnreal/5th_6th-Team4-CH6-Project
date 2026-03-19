// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_CharacterSelect.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/UK_PlayerController_Title.h"
#include "Systems/UK_GameInstance.h"
#include "Systems/Data/UK_SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/OutGame/UK_Out_Loading.h"

UUK_Out_CharacterSelect::UUK_Out_CharacterSelect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUK_Out_CharacterSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ThisClass::OnStartButtonClicked);
	}
}

void UUK_Out_CharacterSelect::OnStartButtonClicked()
{
	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	if ( !GI || !LoadingWidgetClass ) return;

	if ( GI->PersistentLoadingWidget == nullptr )
	{
		UUK_Out_Loading* NewLoading = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);
		if ( NewLoading )
		{
			NewLoading->TargetValue = 0.7f;

			if ( GEngine && GEngine->GameViewport )
			{
				GEngine->GameViewport->AddViewportWidgetContent(NewLoading->TakeWidget(), 0);

				GI->PersistentLoadingWidget = NewLoading;
			}
		}
	}
	else
	{
		GI->PersistentLoadingWidget->TargetValue = 0.7f;
	}

	RemoveFromParent();

	AUK_PlayerController_Title* PC = GetOwningPlayer<AUK_PlayerController_Title>();
	if ( IsValid(PC) )
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ PC ] ()
			{
				if ( IsValid(PC) )
				{
					PC->StartGame();
				}
			}, 0.1f, false);
	}
}
