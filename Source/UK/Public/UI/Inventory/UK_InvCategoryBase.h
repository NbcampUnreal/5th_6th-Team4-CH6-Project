#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "GameplayTagContainer.h"
#include "UK_InvCategoryBase.generated.h"

class UUniformGridPanel;
class UUK_InvSlot;
class UDataTable;

UENUM(BlueprintType)
enum class EInvCategory : uint8
{
	All,
	Weapon,
	Food,
	Material
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategorySlotHovered, const FInventorySlot&, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCategorySlotUnhovered);

UCLASS()
class UK_API UUK_InvCategoryBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inv")
	EInvCategory CategoryType = EInvCategory::All;

	UFUNCTION(BlueprintCallable, Category = "Inv")
	void SetInvArraySlots(const TArray<FInventorySlot>& InAllSlots);

	UFUNCTION(BlueprintCallable, Category = "Inv")
	void AddSlot(int32 AddCount);

	virtual void NativeConstruct() override;
	virtual bool IsItemAllowed(const FInventorySlot& InSlot) const;
	void CreateSlots();
	void UpdateSlots();

	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnCategorySlotHovered OnCategorySlotHovered;

	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnCategorySlotUnhovered OnCategorySlotUnhovered;

	UFUNCTION()
	void HandleSlotHovered(const FInventorySlot& SlotData);

	UFUNCTION()
	void HandleSlotUnhovered();

	UPROPERTY(BlueprintReadOnly, meta = ( BindWidgetOptional ))
	UUniformGridPanel* SlotGrid;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inv")
	TSubclassOf<UUK_InvSlot> InvSlotClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inv")
	TArray<TObjectPtr<UDataTable>> ItemDataTables;

	TArray<FInventorySlot> FilteredSlots;
	TArray<UUK_InvSlot*> SlotWidgets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InvSlot")
	int32 SlotColumns = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InvSlot")
	int32 SlotRows = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvSlotCount")
	int32 CurrentSlot = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvSlotCount")
	int32 MaxSlot = 999;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvDrag")
	bool bAllowSlotDrag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvDrag")
	FGameplayTag WeaponRootTag;
};