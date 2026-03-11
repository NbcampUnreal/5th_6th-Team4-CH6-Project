#include "UI/Inventory/UK_InvSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

//아이템
#include "DataAsset/Data/UK_ItemData.h"
#include "ActorComponent/UK_InventoryComponent.h"
//드래그
#include "UI/Inventory/UK_DraggedItem.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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

FReply UUK_InvSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && !SlotData.isEmpty())
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

	if (SlotData.isEmpty()) return;

	UUK_InvDragDropOperation* DragOp = NewObject<UUK_InvDragDropOperation>();

	if (!DragOp) return;

	DragOp->DraggedSlotData = SlotData;
	DragOp->SourceIndex = SlotIndex;

	// 드래그 비주얼 생성
	if (DraggedItemClass)
	{
		UUK_DraggedItem* DragVisual = CreateWidget<UUK_DraggedItem>(GetWorld(), DraggedItemClass);

		if (DragVisual && ItemDataTable)
		{
			FUK_ItemData* ItemInfo = ItemDataTable->FindRow<FUK_ItemData>(SlotData.ItemID,TEXT("DragVisual"));

			if (ItemInfo)
			{
				UTexture2D* Icon = ItemInfo->ItemIcon.LoadSynchronous();
				DragVisual->SetIcon(Icon);
			}
		}
		DragOp->DefaultDragVisual = DragVisual;
	}
	DragOp->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOp;
}
