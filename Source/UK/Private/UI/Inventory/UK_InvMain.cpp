#include "UI/Inventory/UK_InvMain.h"
#include "UI/Inventory/UK_InvInfo.h"
#include "UI/Inventory/UK_InvUI.h"
#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/WidgetSwitcher.h"

#include "Character/UK_CharacterBase.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_MoneyWidget.h"


void UUK_InvMain::NativeConstruct()
{
	Super::NativeConstruct();

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

	if (TapSystem)
	{
		TapSystem->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if (TapInventory)
	{
		TapInventory->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if (TapMap)
	{
		TapMap->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (CB)
	{
		UUK_InventoryComponent* InvComp = CB->GetInventoryComponent();
		if (MoneyWidget && InvComp)
		{
			MoneyWidget->BindInventoryComponent(InvComp);
		}
	}
}

void UUK_InvMain::TapClicked(UUK_InvTapbutton* ClickTap)
{
	if (!InvSwitcher) return;

	if (ClickTap == TapInventory)
	{
		InvSwitcher->SetActiveWidgetIndex(0);
	}
	else if (ClickTap == TapSystem)
	{
		InvSwitcher->SetActiveWidgetIndex(1);
	}
	else if (ClickTap == TapMap)
	{
		InvSwitcher->SetActiveWidgetIndex(2);
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