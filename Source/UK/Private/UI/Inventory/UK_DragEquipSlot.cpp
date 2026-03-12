#include "UI/Inventory/UK_DragEquipSlot.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Components/Image.h"

#include "DataAsset/Data/UK_ItemData.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "ActorComponent/UK_InventoryComponent.h"


bool UUK_DragEquipSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UUK_InvDragDropOperation* DragOp = Cast<UUK_InvDragDropOperation>(InOperation);

	if (!DragOp) return false;
	if (DragOp->DraggedSlotData.isEmpty()) return false;
	if (!ItemDataTable) return false;

	const FUK_ItemData* ItemInfo = ItemDataTable->FindRow<FUK_ItemData>(DragOp->DraggedSlotData.ItemID, TEXT("UUK_DragEquipSlot::NativeOnDrop"));

	if (!ItemInfo) return false;
	bool bIsWeapon = false;

	if (WeaponRootTag.IsValid() && ItemInfo->ItemTag.IsValid())
	{
		if (ItemInfo->ItemTag.MatchesTag(WeaponRootTag))
		{
			bIsWeapon = true;
		}
	}

	if (!bIsWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("INV : Weapon Only EquipSlot"));//지울거
		return false;
	}

	const bool bSuccess = InventoryComponent->AddWeapon(DragOp->DraggedSlotData.ItemID, EquipIndex);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipSlot : AddWeapon failed"));//지울거
		return false;
	}

	UpdateEquipSlotVisual(); 

	UE_LOG(LogTemp, Warning, TEXT("EquipSlot : ItemID %s -> WeaponSlots[%d]"),*DragOp->DraggedSlotData.ItemID.ToString(),EquipIndex);//지울거

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
	if (!EquipSlot || !InventoryComponent || !ItemDataTable || EquipIndex == INDEX_NONE) return;

	FInventorySlot* WeaponSlotData = InventoryComponent->FindWeaponSlotbyIndex(EquipIndex);
	if (!WeaponSlotData || WeaponSlotData->isEmpty())
	{
		EquipSlot->SetBrushFromTexture(nullptr);
		return;
	}

	const FUK_ItemData* ItemInfo = ItemDataTable->FindRow<FUK_ItemData>(WeaponSlotData->ItemID,	TEXT("UUK_DragEquipSlot::UpdateEquipSlotVisual"));

	if (!ItemInfo)
	{
		EquipSlot->SetBrushFromTexture(nullptr);
		return;
	}

	UTexture2D* IconTexture = ItemInfo->ItemIcon.LoadSynchronous();
	EquipSlot->SetBrushFromTexture(IconTexture);
}
