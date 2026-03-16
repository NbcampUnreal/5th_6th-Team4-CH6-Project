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
	AUK_PlayerController_Title* PlayerController = GetOwningPlayer<AUK_PlayerController_Title>();

	UUK_Out_Loading* Loading = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);
	if ( Loading )
	{
		Loading->TargetValue = 0.7f; // 70% 목표 설정
		Loading->AddToViewport(999); // 가장 앞에 출력

		UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
		if ( GI )
		{
			GI->PersistentLoadingWidget = Loading;
		}
	}

	//UUK_Out_Loading* Loading = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);
	//if ( Loading )
	//{
	//	Loading->TargetValue = 0.7f;
	//	Loading->AddToViewport(999);

	//	// GameInstance에 포인터 저장 (파괴 방지 및 참조 유지)
	//	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	//	if ( GI )
	//	{
	//		GI->PersistentLoadingWidget = Loading;
	//	}
	//}


	RemoveFromParent();

	if ( IsValid(PlayerController) == true )
	{
		//FText ServerIP = ServerIPEditableText->GetText();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ PlayerController ] ()
			{
				if ( IsValid(PlayerController) )
				{
					PlayerController->StartGame();
				}
			}, 0.1f, false);
	}
}
