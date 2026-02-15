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
			// 델리게이트 바인딩 
			StatusComp->HpStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateHealthBar);

			// 리플렉션을 이용해 protected 변수인 Status에 강제 접근
			FStructProperty* StatusProp = FindFieldChecked<FStructProperty>(UStatusComponent::StaticClass(), TEXT("Status"));

			if ( StatusProp )
			{
				// StatusComp 인스턴스 내의 실제 Status 구조체 주소를 가져옴
				const FStatus* StatusPtr = StatusProp->ContainerPtrToValuePtr<FStatus>(StatusComp);
				if ( StatusPtr )
				{
					// 찾은 초기값으로 UpdateHealthBar를 수동 호출하여 UI를 갱신
					UpdateHealthBar(StatusPtr->CurrentHp, StatusPtr->MaxHp);
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

