#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_DraggedItem.h"
#include "UI/Inventory/UK_InvDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "UK_InvSlot.generated.h"

class UImage;
class USizeBox;
class UTextBlock;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvSlotHovered, const FInventorySlot&, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvSlotUnhovered);

UCLASS()
class UK_API UUK_InvSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* QuantitySizeBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemQuantityText;

	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnInvSlotHovered OnSlotHovered;

	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnInvSlotUnhovered OnSlotUnhovered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ExposeOnSpawn = "true")) // ExposeOnSpawn은 객체가 생성되는 그 순간에 바로 값을 꽂아넣는다.
	int32 SlotIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ExposeOnSpawn = "true"))
	FInventorySlot SlotData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ExposeOnSpawn = "true"))
	TArray<TObjectPtr<UDataTable>> ItemDataTables;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void UpdateSlot();

	UPROPERTY(EditAnywhere, Category = "Drag")
	TSubclassOf<UUK_DraggedItem> DraggedItemClass;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag", meta = ( ExposeOnSpawn = "true" ))
	bool bAllowDrag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
	FGameplayTag WeaponRootTag;

	UFUNCTION(BlueprintCallable, Category = "Drag")
	bool IsWeaponItem() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemSlot", meta = ( ExposeOnSpawn = "true" ))
	int32 SourceInventoryIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemSlot", meta = ( ExposeOnSpawn = "true" ))
	TObjectPtr<UUK_InventoryComponent> InventoryComp;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};