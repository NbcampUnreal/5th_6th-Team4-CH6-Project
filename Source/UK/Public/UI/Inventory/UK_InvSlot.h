#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_DraggedItem.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UK_InvSlot.generated.h"

class UImage;
class USizeBox;
class UTextBlock;
class UDataTable;
//info
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvSlotHovered, const FInventorySlot&, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvSlotUnhovered);

UCLASS()
class UK_API UUK_InvSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	//바인드
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* QuantitySizeBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemQuantityText;

	//info
	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnInvSlotHovered OnSlotHovered;

	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnInvSlotUnhovered OnSlotUnhovered;
	
	//인덱스 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ExposeOnSpawn = "true"))
	int32 SlotIndex;
	//슬롯 데이터 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = ( ExposeOnSpawn = "true" ))
	FInventorySlot SlotData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = ( ExposeOnSpawn = "true" ))
	UDataTable* ItemDataTable;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void UpdateSlot();

	UPROPERTY(EditAnywhere, Category = "Drag")
	TSubclassOf<UUK_DraggedItem> DraggedItemClass;

	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;


protected:
	virtual void NativePreConstruct() override;

	//info
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
};
