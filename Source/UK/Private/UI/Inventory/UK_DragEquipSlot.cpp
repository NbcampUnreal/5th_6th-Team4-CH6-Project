#include "UI/Inventory/UK_DragEquipSlot.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Components/Image.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_ItemTableHelper.h"

bool UUK_DragEquipSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UUK_InvDragDropOperation* DragOp = Cast<UUK_InvDragDropOperation>(InOperation);

	if (!DragOp) return false;
	if (DragOp->DraggedSlotData.isEmpty()) return false;

	FUK_ItemTableRowView ItemInfo;

	if (!UK_ItemTableHelper::FindItemData(ItemDataTables, DragOp->DraggedSlotData.ItemID, ItemInfo))
	{
		return false;
	}

	bool bIsWeapon = false;

	if (WeaponRootTag.IsValid() && ItemInfo.ItemTag.IsValid())
	{
		if (ItemInfo.ItemTag.MatchesTag(WeaponRootTag))
		{
			bIsWeapon = true;
		}
	}
	const bool bSuccess = InventoryComponent->AddWeaponFromInventoryIndex(
		DragOp->DraggedSlotData.ItemID,
		EquipIndex,
		DragOp->SourceInventoryIndex);

	UpdateEquipSlotVisual();
	return true;
}

void UUK_DragEquipSlot::BindInventory(UUK_InventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent) return;

	InventoryComponent = InInventoryComponent;
	UpdateEquipSlotVisual();
}

void UUK_DragEquipSlot::UpdateEquipSlotVisual()
{
	if (!EquipSlot || !InventoryComponent || EquipIndex == INDEX_NONE) return;

	FInventorySlot* WeaponSlotData = InventoryComponent->FindWeaponSlotbyIndex(EquipIndex);
	if (!WeaponSlotData || WeaponSlotData->isEmpty())
	{
		EquipSlot->SetBrushFromTexture(nullptr);
		return;
	}

	FUK_ItemTableRowView ItemInfo;
	if (!UK_ItemTableHelper::FindItemData(ItemDataTables, WeaponSlotData->ItemID, ItemInfo))
	{
		EquipSlot->SetBrushFromTexture(nullptr);
		return;
	}

	if (UTexture2D* IconTexture = ItemInfo.ItemIcon.LoadSynchronous())
	{
		EquipSlot->SetBrushFromTexture(IconTexture);
	}
	else
	{
		EquipSlot->SetBrushFromTexture(nullptr);
	}
}