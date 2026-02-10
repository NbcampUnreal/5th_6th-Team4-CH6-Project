#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_InvSlot.generated.h"

class UImage;
class USizeBox;
class UTextBlock;
class UDataTable;

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

	UFUNCTION(BlueprintCallable)
	void SetSlotData(int32 InSlotIndex, const FInventorySlot& InSlotData);

protected:
	virtual void NativeConstruct() override;

	
};
