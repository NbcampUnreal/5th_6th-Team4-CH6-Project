#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_InvCategoryBase.generated.h"

//class UUniformGridPanel;
//class UUK_InvSlot;


UCLASS()
class UK_API UUK_InvCategoryBase : public UUserWidget
{
	GENERATED_BODY()
	
//public:
//
//	UFUNCTION(BlueprintCallable, Category = "SlotPage")
//	void SlotArry(const TArray<FInventorySlot>& InALLSlots);
//
//	virtual void NativeConstruct() override;
//	virtual FReply NativeOnMouseWheel(
//		const FGeometry& InGeometry,
//		const FPointerEvent& InMouseEvent
//	) override;
//
//	virtual bool bIsItemAllowed(const FInventorySlot& Slot) const;
//
//	void CreateSlots();
//	void UpdateSlots();
//	void ChangePage(int32 NewPage);
//
//	int32 GetSlotCountPerPage() const;
//
//	//바인드
//	UPROPERTY(meta = (BindWidget))
//	UUniformGridPanel* SlotGrid;
//
//	UPROPERTY(EditDefaultsOnly, Category = "Inv")
//	TSubclassOf<UUK_InvSlot> SlotWidgetClass;
//
//	UPROPERTY(EditDefaultsOnly, Category = "Inv")
//	UDataTable* ItemDataTable;
//
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inv_SlotPage")
//	int32 SlotColumns = 7;
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inv_SlotPage")
//	int32 SlotRows = 4;
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inv_SlotPage")
//	int32 Maxpages = 5;
//
//	int32 CurrentPage = 0;
//
//	TArray<FInventorySlot> FilteredSlots;
//	TArray<UUK_InvSlot*> SlotWidgets;
};
