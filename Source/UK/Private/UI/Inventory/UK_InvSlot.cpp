#include "UI/Inventory/UK_InvSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

//아이템
#include "DataAsset/Data/UK_ItemData.h"
#include "ActorComponent/UK_InventoryComponent.h"

void UUK_InvSlot::NativePreConstruct()
{
	Super::NativePreConstruct();
	UpdateSlot();
}

void UUK_InvSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!SlotData.isEmpty())
		OnSlotHovered.Broadcast(SlotData);
}

void UUK_InvSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	OnSlotUnhovered.Broadcast();
}

void UUK_InvSlot::UpdateSlot()
{
	bool bIsEmpty = SlotData.isEmpty();

	ESlateVisibility TargetVisibility = bIsEmpty ? ESlateVisibility::Hidden : ESlateVisibility::Visible;

	if (QuantitySizeBox)
	{
		QuantitySizeBox->SetVisibility(TargetVisibility);
	}
	if(ItemImage)
	{
		ItemImage->SetVisibility(TargetVisibility);
	}

	if (bIsEmpty) return;

	if (ItemQuantityText)
	{
		ItemQuantityText->SetText(FText::AsNumber(SlotData.Quantity));
	}

	if (ItemDataTable && !SlotData.ItemID.IsNone())
	{

		FUK_ItemData* ItemInfo = ItemDataTable->FindRow<FUK_ItemData>(SlotData.ItemID, TEXT("UI_UpdateSlot"));

		if (ItemInfo && ItemImage)
		{

			UTexture2D* IconTexture = ItemInfo->ItemIcon.LoadSynchronous();
			if (IconTexture)
			{
				ItemImage->SetBrushFromTexture(IconTexture);
			}
		}
	}
}
