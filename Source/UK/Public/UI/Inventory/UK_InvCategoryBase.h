#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_InvCategoryBase.generated.h"

class UUniformGridPanel;
class UUK_InvSlot;


UCLASS()
class UK_API UUK_InvCategoryBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Inv")
	void SetInvArraySlots(const TArray<FInventorySlot>& InAllSlots);

	UFUNCTION(BlueprintCallable, Category = "Inv")
	void AddSlot(int32 AddCount);

	virtual void NativeConstruct() override;
	virtual bool IsItemAllowed(const FInventorySlot& InSlot) const;

	void CreateSlots();
	void UpdateSlots();

	//바인드
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UUniformGridPanel* SlotGrid;

	//인벤토리 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inv")
	TSubclassOf<UUK_InvSlot> InvSlotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inv")
	UDataTable* ItemDataTable;

	//==슬롯 갯수 한 페이지당==
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InvSlot")
	int32 SlotColumns = 7; //슬롯 배열 가로
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InvSlot")
	int32 SlotRows = 4; //슬롯 배열 세로
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvSlotCount")
	int32 CurrentSlot = 7; //슬롯 현재 갯수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvSlotCount")
	int32 MaxSlot = 999; //슬롯 최대 갯수
	//=========================
	
	//인벤토리 전체 슬롯 배열
	TArray<FInventorySlot> FilteredSlots;
	TArray<UUK_InvSlot*> SlotWidgets;
};
