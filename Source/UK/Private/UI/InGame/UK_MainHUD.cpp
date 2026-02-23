#include "UI/InGame/UK_MainHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/Inventory/UK_InvMain.h"
#include "Kismet/GameplayStatics.h"

void UUK_MainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if ( PlayerPawn )
	{
		UStatusComponent* StatusComp = PlayerPawn->FindComponentByClass<UStatusComponent>();
		if ( StatusComp )
		{
			// HP, MP, 레벨, 스테미나 바인딩
			StatusComp->HpStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateHealthBar);
			StatusComp->MpStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateMpBar);
			StatusComp->LevelStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateLevel);
			StatusComp->StaminaStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateStaminaBar);

			FStructProperty* StatusProp = FindFieldChecked<FStructProperty>(UStatusComponent::StaticClass(), TEXT("Status"));
			if ( StatusProp )
			{
				const FStatus* StatusPtr = StatusProp->ContainerPtrToValuePtr<FStatus>(StatusComp);
				if ( StatusPtr )
				{
					UpdateHealthBar(StatusPtr->CurrentHp, StatusPtr->MaxHp);
					UpdateMpBar(StatusPtr->CurrentMp, StatusPtr->MaxMp);
					UpdateLevel(StatusPtr->Level);
					UpdateStaminaBar(StatusPtr->CurrentStamina, StatusPtr->MaxStamina); 
				}
			}
		}
	}

	if ( InventoryButton )
	{
		InventoryButton->OnClicked.AddDynamic(this, &UUK_MainHUD::OnInventoryButtonClicked);
	}
}

void UUK_MainHUD::UpdateHealthBar(float CurrentHp, float MaxHp)
{
	if ( HealthBar && MaxHp > 0.f )
	{
		// 0.0 ~ 1.0 사이의 퍼센트 값으로 변환하여 반영
		HealthBar->SetPercent(CurrentHp / MaxHp);
	}

	// 현재 체력 업데이트 
	if ( CurrentHealthText )
	{
		CurrentHealthText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentHp)));
	}

	// 최대 체력 업데이트 
	if ( MaxHealthText )
	{
		MaxHealthText->SetText(FText::AsNumber(FMath::FloorToInt(MaxHp)));
	}
}

void UUK_MainHUD::UpdateMpBar(float CurrentMp, float MaxMp)
{
	if ( MpBar && MaxMp > 0.f )
	{
		MpBar->SetPercent(CurrentMp / MaxMp);
	}

	if ( CurrentMpText )
	{
		CurrentMpText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentMp)));
	}

	if ( MaxMpText )
	{
		MaxMpText->SetText(FText::AsNumber(FMath::FloorToInt(MaxMp)));
	}
}

void UUK_MainHUD::UpdateLevel(int32 NewLevel)
{
	if ( LevelText )
	{
		LevelText->SetText(FText::AsNumber(NewLevel));
	}
}

void UUK_MainHUD::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
{
	if ( WBP_Stamina && MaxStamina > 0.f )
	{
		float PercentValue = CurrentStamina / MaxStamina;
        FString Command = FString::Printf(TEXT("Setpercent %f"), PercentValue);
        WBP_Stamina->CallFunctionByNameWithArguments(*Command, *GLog, nullptr, true);
	}
}

void UUK_MainHUD::OnInventoryButtonClicked()
{
	if ( !InvMainClass ) return;

	// 인벤토리 위젯이 생성되지 않았다면 생성
	if ( !InvMainWidget )
	{
		InvMainWidget = CreateWidget<UUK_InvMain>(GetWorld(), InvMainClass);
	}

	if ( InvMainWidget )
	{
		if ( !InvMainWidget->IsInViewport() )
		{
			// 화면에 추가
			InvMainWidget->AddToViewport();

			// 마우스 커서 활성화 및 입력 모드 변경
			APlayerController* PC = GetOwningPlayer();
			if ( PC )
			{
				PC->SetShowMouseCursor(true);
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(InvMainWidget->TakeWidget());
				PC->SetInputMode(InputMode);
			}
		}
		else
		{
			// 이미 열려있다면 닫기 (토글 방식)
			InvMainWidget->RemoveFromParent();

			APlayerController* PC = GetOwningPlayer();
			if ( PC )
			{
				PC->SetShowMouseCursor(false);
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}
}

