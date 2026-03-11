#include "UI/Inventory/UK_DragEquipSlot.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Components/Image.h"
#include "DataAsset/Data/UK_ItemData.h"

#include "Character/Weapon/UK_WeaponBase.h"

bool UUK_DragEquipSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UE_LOG(LogTemp, Warning, TEXT("INV : Drop Detected")); //지울거

	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UUK_InvDragDropOperation* DragOp = Cast<UUK_InvDragDropOperation>(InOperation);

	if (!DragOp) return false;
	if (DragOp->DraggedSlotData.isEmpty()) return false;
	

	UE_LOG(LogTemp, Warning, TEXT("INV : Equip ItemID %s"), *DragOp->DraggedSlotData.ItemID.ToString());//지울거

	return true;
}
