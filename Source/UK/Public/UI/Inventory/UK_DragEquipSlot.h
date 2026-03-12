#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_DragEquipSlot.generated.h"

class UImage;;
class UDataTable;
class UUK_InventoryComponent;

UCLASS()
class UK_API UUK_DragEquipSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* EquipSlot;

	//드래그
	//EquipSlot 번호
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip", meta = ( ExposeOnSpawn = "true" ))
	int32 EquipIndex = INDEX_NONE;

	//아이템 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip", meta = ( ExposeOnSpawn = "true" ))
	UDataTable* ItemDataTable;

	//인벤토리 참조
	UPROPERTY(BlueprintReadWrite, Category = "Equip")
	UUK_InventoryComponent* InventoryComponent;

	//무기 루트 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip")
	FGameplayTag WeaponRootTag;

	UFUNCTION(BlueprintCallable)
	void BindInventory(UUK_InventoryComponent* InInventoryComponent);

	UFUNCTION(BlueprintCallable)
	void UpdateEquipSlotVisual();

protected:

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry, 
		const FDragDropEvent& InDragDropEvent, 
		UDragDropOperation* InOperation) override;
	
};
