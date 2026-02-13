#include "UI/InGame/UK_MainHUD.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "UI/Inventory/UK_InvMain.h"
#include "Kismet/GameplayStatics.h"

void UUK_MainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 플레이어 캐릭터 가져오기
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if ( PlayerPawn )
	{
		// 캐릭터에서 StatusComponent 찾기
		UStatusComponent* StatusComp = PlayerPawn->FindComponentByClass<UStatusComponent>();
		if ( StatusComp )
		{
			// 3. 델리게이트 바인딩 
			StatusComp->HpStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateHealthBar);

		}
	}

	//인벤토리 버튼 클릭 이벤트 바인딩
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

