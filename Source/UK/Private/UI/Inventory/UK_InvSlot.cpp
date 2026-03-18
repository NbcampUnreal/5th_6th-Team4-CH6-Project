#include "UI/Inventory/UK_InvSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "UI/Inventory/UK_DraggedItem.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Inventory/UK_ItemTableHelper.h"

void UUK_InvSlot::NativePreConstruct()
{
	Super::NativePreConstruct();
	UpdateSlot();
}

void UUK_InvSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!SlotData.isEmpty())
	{
		OnSlotHovered.Broadcast(SlotData);
	}
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

	if (ItemImage)
	{
		ItemImage->SetVisibility(TargetVisibility);
	}

	if ( bIsEmpty ) return;

	if (ItemQuantityText)
	{
		ItemQuantityText->SetText(FText::AsNumber(SlotData.Quantity));
	}

	FUK_ItemTableRowView ItemInfo;
	if (UK_ItemTableHelper::FindItemData(ItemDataTables, SlotData.ItemID, ItemInfo) && ItemImage)
	{
		if (UTexture2D* IconTexture = ItemInfo.ItemIcon.LoadSynchronous())
		{
			ItemImage->SetBrushFromTexture(IconTexture);
		}
	}
}

FReply UUK_InvSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)
		&& !SlotData.isEmpty()
		&& bAllowDrag
		&& IsWeaponItem())
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent,
			this,
			EKeys::LeftMouseButton
		).NativeReply;
	}

	return FReply::Unhandled();
}

void UUK_InvSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (SlotData.isEmpty() || !bAllowDrag || !IsWeaponItem()) return;

	UUK_InvDragDropOperation* DragOp = NewObject<UUK_InvDragDropOperation>();
	if (!DragOp) return;

	DragOp->DraggedSlotData = SlotData;
	DragOp->SourceIndex = SlotIndex;
	DragOp->SourceInventoryIndex = SourceInventoryIndex;

	if (DraggedItemClass)
	{
		UUK_DraggedItem* DragVisual = CreateWidget<UUK_DraggedItem>(GetWorld(), DraggedItemClass);

		FUK_ItemTableRowView ItemInfo;
		if (DragVisual && UK_ItemTableHelper::FindItemData(ItemDataTables, SlotData.ItemID, ItemInfo))
		{
			if (UTexture2D* Icon = ItemInfo.ItemIcon.LoadSynchronous())
			{
				DragVisual->SetIcon(Icon);
			}
		}

		DragOp->DefaultDragVisual = DragVisual;
	}

	DragOp->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOp;
}

bool UUK_InvSlot::IsWeaponItem() const
{
	FUK_ItemTableRowView ItemInfo;
	if (!UK_ItemTableHelper::FindItemData(ItemDataTables, SlotData.ItemID, ItemInfo))
	{
		return false;
	}

	if (WeaponRootTag.IsValid() && ItemInfo.ItemTag.IsValid())
	{
		return ItemInfo.ItemTag.MatchesTag(WeaponRootTag);
	}

	return false;
}