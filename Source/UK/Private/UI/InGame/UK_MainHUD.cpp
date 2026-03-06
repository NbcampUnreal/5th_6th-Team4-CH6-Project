#include "UI/InGame/UK_MainHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/Inventory/UK_InvMain.h"
#include "Kismet/GameplayStatics.h"

//추가
#include "UI/Inventory/UK_ItemNotify.h"
#include "Components/VerticalBox.h"
#include "Character/UK_CharacterBase.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "Blueprint/UserWidget.h"
//끝
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

			FStructProperty* StatusProp = FindFieldChecked<FStructProperty>(UStatusComponent::StaticClass(), TEXT("Status"));
			if ( StatusProp )
			{
				const FStatus* StatusPtr = StatusProp->ContainerPtrToValuePtr<FStatus>(StatusComp);
				if ( StatusPtr )
				{
					UpdateHealthBar(StatusPtr->CurrentHp, StatusPtr->MaxHp);
					UpdateMpBar(StatusPtr->CurrentMp, StatusPtr->MaxMp);
					UpdateLevel(StatusPtr->Level);
					//UpdateStaminaBar(StatusPtr->CurrentStamina, StatusPtr->MaxStamina); 
				}
			}
		}
	}

	if ( InventoryButton )
	{
		InventoryButton->OnClicked.AddDynamic(this, &UUK_MainHUD::OnInventoryButtonClicked);
	}

	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (CB)
	{
		InvComp = CB->GetInventoryComponent();
		if (InvComp)
		{
			InvComp->OnItemAdded.AddDynamic(this, &UUK_MainHUD::ShowItemNotify);
		}
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

void UUK_MainHUD::ShowItemNotify(FName ItemID, int32 Amount)
{
	if ( !VB_ItemNotify ) return;
	if ( !ItemNotifyClass ) return;

	//수정: IsInViewport() 쓰지 말고, VB에 붙어있는지(Parent 존재)로 체크
	if ( TObjectPtr<UUK_ItemNotify>* Found = ActiveNotifyMap.Find(ItemID) ) //기존
	{
		if ( Found->Get() && Found->Get()->GetParent() != nullptr ) //수정
		{
			Found->Get()->AddAmount(Amount); //기존
			return; //기존
		}
		else
		{
			ActiveNotifyMap.Remove(ItemID); //기존(정리)
		}
	}

	UUK_ItemNotify* Notify = CreateWidget<UUK_ItemNotify>(GetWorld(), ItemNotifyClass);
	if ( !Notify ) return;

	Notify->ItemDataTable = ItemDataTable;
	Notify->NotifyItem(ItemID, Amount);

	VB_ItemNotify->AddChild(Notify);

	ActiveNotifyMap.Add(ItemID, Notify); //기존

	NotifyChildren();
}

void UUK_MainHUD::NotifyChildren()
{
	if (!VB_ItemNotify) return;

	while (VB_ItemNotify->GetChildrenCount() > MaxNotifyCount)
	{
		UWidget* Oldest = VB_ItemNotify->GetChildAt(0);

		if (UUK_ItemNotify* OldNotify = Cast<UUK_ItemNotify>(Oldest))
		{
			ActiveNotifyMap.Remove(OldNotify->GetItemID());
		}

		VB_ItemNotify->RemoveChildAt(0);
	}
}

