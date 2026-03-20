#include "UI/InGame/UK_MainHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/Inventory/UK_InvMain.h"
#include "Kismet/GameplayStatics.h"

//추가
#include "UI/Inventory/UK_ItemNotify.h"
#include "Components/VerticalBox.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Character/UK_CharacterBase.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
//끝
void UUK_MainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	SetInventoryNewVisible(false);

	PlayerPawn = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (PlayerPawn)
	{
		ASC = PlayerPawn->GetAbilitySystemComponent();
		if (ASC)
		{
			FOnAttributeChangeData Data;
			// 		//UpdateStaminaBar(StatusPtr->CurrentStamina, StatusPtr->MaxStamina); 
			// HP 초기값 세팅 및 바인딩
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute()).
				AddUObject(this, &UUK_MainHUD::UpdateHealthBar);
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetHealthAttribute()).
				AddUObject(this, &UUK_MainHUD::UpdateHealthBar);

			// MP 바인딩
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetMaxMpAttribute()).
				AddUObject(this, &UUK_MainHUD::UpdateMpBar);
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute()).
				AddUObject(this, &UUK_MainHUD::UpdateMpBar);

			// 레벨 바인딩
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetMaxLevelAttribute()).
				AddUObject(this, &UUK_MainHUD::UpdateLevel);
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetLevelAttribute()).
				AddUObject(this, &UUK_MainHUD::UpdateLevel);
			// AttributeSet->MpStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateMpBar);

			// 레벨 초기값 세팅 및 바인딩
			Data.NewValue = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxLevelAttribute());
			UpdateLevel(Data);
			Data.NewValue = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetLevelAttribute());
			UpdateLevel(Data);

			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetMaxLevelAttribute()).
			     AddUObject(this, &UUK_MainHUD::UpdateLevel);
			ASC->GetGameplayAttributeValueChangeDelegate(UUK_PlayerStatusAttributeSet::GetLevelAttribute()).
			     AddUObject(this, &UUK_MainHUD::UpdateLevel);
			// AttributeSet->LevelStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateLevel);
			//
			// 스테미나 바인딩
			// FStructProperty* StatusProp = FindFieldChecked<FStructProperty>(UStatusComponent::StaticClass(), TEXT("Status"));
			// if ( StatusProp )
			// {
			// 	const FStatus* StatusPtr = StatusProp->ContainerPtrToValuePtr<FStatus>(StatusComp);
			// 	if ( StatusPtr )
			// 	{
			// 		UpdateHealthBar(StatusPtr->CurrentHp, StatusPtr->MaxHp);
			// 		UpdateMpBar(StatusPtr->CurrentMp, StatusPtr->MaxMp);
			// 		UpdateLevel(StatusPtr->Level);
			// 		//UpdateStaminaBar(StatusPtr->CurrentStamina, StatusPtr->MaxStamina); 
			// 	}
			// }
			FTimerHandle InitTimer;
			GetWorld()->GetTimerManager().SetTimer(InitTimer, this, &UUK_MainHUD::RefreshAllStatus, 0.4f, false);
		}
	}

	if (InventoryButton)
	{
		InventoryButton->OnClicked.AddDynamic(this, &UUK_MainHUD::OnInventoryButtonClicked);
	}

	if (PlayerPawn)
	{
		InvComp = PlayerPawn->GetInventoryComponent();
		if (InvComp)
		{
			InvComp->OnItemAdded.AddDynamic(this, &UUK_MainHUD::ShowItemNotify);
			InvComp->OnItemAdded.AddDynamic(this, &UUK_MainHUD::HandleItemAdded_ShowNew);
		}
	}
}

void UUK_MainHUD::UpdateHealthBar(const FOnAttributeChangeData& Data)
{
	if (IsValid(PlayerPawn) == false || IsValid(ASC) == false)
		return;

	if (Data.Attribute == UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute())
	{
		float CurrentHealth =
			ASC->GetNumericAttribute(
				UUK_PlayerStatusAttributeSet::GetHealthAttribute());
		if (HealthBar && Data.NewValue > 0.f)
		{
			// 0.0 ~ 1.0 사이의 퍼센트 값으로 변환하여 반영
			HealthBar->SetPercent(CurrentHealth / Data.NewValue);
			MaxHealthText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	if (Data.Attribute == UUK_PlayerStatusAttributeSet::GetHealthAttribute())
	{
		float MaxHealth =
			ASC->GetNumericAttribute(
				UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute());
		if (HealthBar && Data.NewValue >= 0.f)
		{
			// 0.0 ~ 1.0 사이의 퍼센트 값으로 변환하여 반영
			HealthBar->SetPercent(Data.NewValue / MaxHealth);
			CurrentHealthText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
}

void UUK_MainHUD::UpdateMpBar(const FOnAttributeChangeData& Data)
{
	if (IsValid(PlayerPawn) == false || IsValid(ASC) == false)
		return;

	if (Data.Attribute == UUK_PlayerStatusAttributeSet::GetMaxMpAttribute())
	{
		float CurrentMp =
			ASC->GetNumericAttribute(
				UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute());
		if (MpBar && Data.NewValue > 0.f)
		{
			// 0.0 ~ 1.0 사이의 퍼센트 값으로 변환하여 반영
			MpBar->SetPercent(CurrentMp / Data.NewValue);
			MaxMpText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	if (Data.Attribute == UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute())
	{
		float MaxMp =
			ASC->GetNumericAttribute(
				UUK_PlayerStatusAttributeSet::GetMaxMpAttribute());
		if (MpBar && Data.NewValue >= 0.f)
		{
			// 0.0 ~ 1.0 사이의 퍼센트 값으로 변환하여 반영
			MpBar->SetPercent(Data.NewValue / MaxMp);
			CurrentMpText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	// if (MpBar && MaxMp > 0.f)
	// {
	// 	MpBar->SetPercent(CurrentMp / MaxMp);
	// }
	//
	// if (CurrentMpText)
	// {
	// }
	//
	// if (MaxMpText)
	// {
	// }
}

void UUK_MainHUD::UpdateLevel(const FOnAttributeChangeData& Data)
{
	if (IsValid(PlayerPawn) == false || IsValid(ASC) == false)
		return;

	if (Data.Attribute == UUK_PlayerStatusAttributeSet::GetMaxLevelAttribute())
	{
	}
	if (Data.Attribute == UUK_PlayerStatusAttributeSet::GetLevelAttribute())
	{
		if (LevelText)
		{
			LevelText->SetText(FText::AsNumber(Data.NewValue));
		}
	}
	// if (LevelText)
	// {
	// 	LevelText->SetText(FText::AsNumber(NewLevel));
	// }
}

void UUK_MainHUD::OnInventoryButtonClicked()
{
	ToggleInventory();
}

void UUK_MainHUD::HandleItemAdded_ShowNew(FName ItemID, int32 Amount)
{
	SetInventoryNewVisible(true);
}

void UUK_MainHUD::SetInventoryNewVisible(bool bVisible)
{
	if (!NewText) return;

	NewText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UUK_MainHUD::RefreshAllStatus()
{
	if (!ASC) return;

	FOnAttributeChangeData Data;

	// HP 강제 갱신
	Data.Attribute = UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute();
	Data.NewValue = ASC->GetNumericAttribute(Data.Attribute);
	UpdateHealthBar(Data);
	Data.Attribute = UUK_PlayerStatusAttributeSet::GetHealthAttribute();
	Data.NewValue = ASC->GetNumericAttribute(Data.Attribute);
	UpdateHealthBar(Data);

	// MP 강제 갱신
	Data.Attribute = UUK_PlayerStatusAttributeSet::GetMaxMpAttribute();
	Data.NewValue = ASC->GetNumericAttribute(Data.Attribute);
	UpdateMpBar(Data);
	Data.Attribute = UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute();
	Data.NewValue = ASC->GetNumericAttribute(Data.Attribute);
	UpdateMpBar(Data);

	// Level 강제 갱신
	Data.Attribute = UUK_PlayerStatusAttributeSet::GetLevelAttribute();
	Data.NewValue = ASC->GetNumericAttribute(Data.Attribute);
	UpdateLevel(Data);
}

void UUK_MainHUD::ShowItemNotify(FName ItemID, int32 Amount)
{
	if (!VB_ItemNotify) return;
	if (!ItemNotifyClass) return;

	if (TObjectPtr<UUK_ItemNotify>* Found = ActiveNotifyMap.Find(ItemID))
	{
		if (Found->Get() && Found->Get()->GetParent() != nullptr)
		{
			Found->Get()->AddAmount(Amount);
			return;
		}
		else
		{
			ActiveNotifyMap.Remove(ItemID);
		}
	}

	UUK_ItemNotify* Notify = CreateWidget<UUK_ItemNotify>(GetWorld(), ItemNotifyClass);
	if (!Notify) return;

	Notify->ItemDataTables = ItemDataTables;
	Notify->NotifyItem(ItemID, Amount);

	VB_ItemNotify->AddChild(Notify);

	ActiveNotifyMap.Add(ItemID, Notify);

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


void UUK_MainHUD::OpenInventory()
{
	if (!InvMainClass) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (!InvMainWidget)
	{
		InvMainWidget = CreateWidget<UUK_InvMain>(GetWorld(), InvMainClass);

		
		if (InvMainWidget)
		{
			InvMainWidget->OwnerMainHUD = this;
		}
	}

	if (InvMainWidget)
	{
		if (!InvMainWidget->IsInViewport())
		{
			InvMainWidget->AddToViewport();
			SetInventoryNewVisible(false);

			PC->SetShowMouseCursor(true);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(InvMainWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PC->SetInputMode(InputMode);

			if (PlayerPawn)
			{
				PlayerPawn->GetCharacterMovement()->StopMovementImmediately();
			}
		}
	}
}

void UUK_MainHUD::CloseInventory()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (InvMainWidget && InvMainWidget->IsInViewport())
	{
		InvMainWidget->RemoveFromParent();
	}

	PC->SetShowMouseCursor(false);

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
}

void UUK_MainHUD::ToggleInventory()
{
	if ( InvMainWidget && InvMainWidget->IsInViewport() )
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}
