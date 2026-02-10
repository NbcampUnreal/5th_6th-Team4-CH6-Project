#include "UI/Inventory/UK_InvSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

//아이템
#include "DataAsset/Data/UK_ItemData.h"
#include "ActorComponent/UK_InventoryComponent.h"

void UUK_InvSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUK_InvSlot::UpdateSlot()
{
	//지울거
	UE_LOG(LogTemp, Warning, TEXT("[UpdateSlot] Index=%d ID=%s Qty=%d  Table=%s  Img=%d Text=%d"),
		SlotIndex,
		*SlotData.ItemID.ToString(),
		SlotData.Quantity,
		*GetNameSafe(ItemDataTable),
		ItemImage != nullptr,
		ItemQuantityText != nullptr);

	if ( !ItemDataTable )
	{
		UE_LOG(LogTemp, Error, TEXT("[UpdateSlot] ItemDataTable NULL")); 
		return;
	}
	// ==
	bool bIsEmpty = SlotData.isEmpty() || SlotData.ItemID.IsNone();
	ESlateVisibility TargetVisibility = bIsEmpty ? ESlateVisibility::Hidden : ESlateVisibility::Visible;

	if ( QuantitySizeBox ) QuantitySizeBox->SetVisibility(TargetVisibility);
	if ( ItemImage ) ItemImage->SetVisibility(TargetVisibility);

	if ( bIsEmpty ) return;

	if ( ItemQuantityText )
	{
		ItemQuantityText->SetText(FText::AsNumber(SlotData.Quantity));
	}
	if ( ItemDataTable )
	{
		FUK_ItemData* ItemInfo = ItemDataTable->FindRow<FUK_ItemData>(SlotData.ItemID, TEXT("UI_UpdateSlot"));
		UE_LOG(LogTemp, Warning, TEXT("[UpdateSlot] FindRow=%s"), ItemInfo ? TEXT("OK") : TEXT("FAIL")); //지울거 

		if ( ItemInfo && ItemImage )
		{
			UTexture2D* IconTexture = ItemInfo->ItemIcon.LoadSynchronous();
			UE_LOG(LogTemp, Warning, TEXT("[UpdateSlot] IconTexture=%s"), *GetNameSafe(IconTexture)); //지우럭

			if ( IconTexture )
			{
				ItemImage->SetBrushFromTexture(IconTexture);
			}
		}
	}
}

void UUK_InvSlot::SetSlotData(int32 InSlotIndex, const FInventorySlot& InSlotData)
{
	UE_LOG(LogTemp, Warning, TEXT("SetSlotData Called - Index: %d, ItemID: %s, Qty: %d"),
		InSlotIndex,
		*InSlotData.ItemID.ToString(),
		InSlotData.Quantity);

	SlotIndex = InSlotIndex;
	SlotData = InSlotData;

	UpdateSlot();
}
