#include "UI/Inventory/UK_InvMain.h"
#include "UI/Inventory/UK_InvInfo.h"
#include "UI/Inventory/UK_InvUI.h"
#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/WidgetSwitcher.h"

#include "Character/UK_CharacterBase.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_MoneyWidget.h"

#include "Components/Button.h"
#include "UI/InGame/UK_MainHUD.h"
#include "InputCoreTypes.h"
#include "Input/Reply.h"
#include "Engine/Engine.h"
#include "Character/UK_PlayerController.h"

FReply UUK_InvMain::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if ( InKeyEvent.GetKey() == EKeys::Escape )
	{
		if (AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()))
		{
			PC->CloseInventoryUI();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UUK_InvMain::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
	{
		if ( InKeyEvent.GetKey() == EKeys::Tab )
		{
			PC->OpenSettingAndCloseOtherUI();
			return FReply::Handled();
		}
	}

	if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
	{
		if ( InKeyEvent.GetKey() == EKeys::U )
		{
			PC->WeaponCrafting_UI();
			return FReply::Handled();
		}
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UUK_InvMain::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	if (InvInfo)
	{
		InvInfo->ItemDataTables = ItemDataTables;
		InvInfo->SetVisibility(ESlateVisibility::Hidden);
	}

	if (InvUI)
	{
		InvUI->ItemDataTables = ItemDataTables;
		InvUI->ApplyItemDataTables();

		InvUI->OnInvSlotPreview.AddDynamic(this, &UUK_InvMain::OnPreviewSlot);
		InvUI->OnInvSlotPreviewCleared.AddDynamic(this, &UUK_InvMain::OnPreviewCleared);
	}

	if (TapState )
	{
		TapState->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if (TapInventory)
	{
		TapInventory->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if (TapMap)
	{
		TapMap->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UUK_InvMain::OnCloseButtonClicked);
	}

	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if ( CB )
	{
		UUK_InventoryComponent* InvComp = CB->GetInventoryComponent();
		if ( InvComp )
		{
			//돈 위젯 바인딩
			if ( MoneyWidget )
			{
				MoneyWidget->BindInventoryComponent(InvComp);
			}

			InvComp->OnInventoryUpdate.AddDynamic(this, &UUK_InvMain::RefreshInventoryUI);
		}
	}
}

void UUK_InvMain::TapClicked(UUK_InvTapbutton* ClickTap)
{
	if (!InvSwitcher) return;

	if (ClickTap == TapInventory)
	{
		SetMainTab(EMainTab::Inventory);
	}
	else if (ClickTap == TapState)
	{
		SetMainTab(EMainTab::System);
	}
	else if (ClickTap == TapMap)
	{
		SetMainTab(EMainTab::Map);
	}
}

void UUK_InvMain::OnPreviewSlot(const FInventorySlot& SlotData)
{
	if (!InvInfo) return;

	TArray<UDataTable*> RawTables;
	RawTables.Reserve(ItemDataTables.Num());

	for (UDataTable* Table : ItemDataTables)
	{
		RawTables.Add(Table);
	}

	InvInfo->SlotMouse(RawTables, SlotData, 24.f, 24.f);
}

void UUK_InvMain::OnPreviewCleared()
{
	if (!InvInfo) return;
	InvInfo->HideToolInfo();
}

void UUK_InvMain::SetMainTab(EMainTab NewTab)
{
	if (!InvSwitcher) return;
	InvSwitcher->SetActiveWidgetIndex(static_cast<int32>(NewTab));
}

void UUK_InvMain::OnCloseButtonClicked()
{
	if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
	{
		PC->CloseInventoryUI(); //수정
	}
}

void UUK_InvMain::CloseInvMain()
{
	if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
	{
		PC->CloseInventoryUI(); //수정
	}
	else
	{
		RemoveFromParent();
	}
}

void UUK_InvMain::RefreshInventoryUI()
{
	if ( InvUI )
	{
		InvUI->ApplyItemDataTables();
	}
}
