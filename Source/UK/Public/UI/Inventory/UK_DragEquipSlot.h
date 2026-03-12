#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_DragEquipSlot.generated.h"

class UImage;
class UDataTable;
class UUK_InventoryComponent;

UCLASS()
class UK_API UUK_DragEquipSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* EquipSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip", meta = ( ExposeOnSpawn = "true" ))
	int32 EquipIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip", meta = ( ExposeOnSpawn = "true" ))
	TArray<TObjectPtr<UDataTable>> ItemDataTables;

	UPROPERTY(BlueprintReadWrite, Category = "Equip")
	UUK_InventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip")
	FGameplayTag WeaponRootTag;

	UFUNCTION(BlueprintCallable)
	void BindInventory(UUK_InventoryComponent* InInventoryComponent);

	UFUNCTION(BlueprintCallable)
	void UpdateEquipSlotVisual();

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry,const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation) override;
};