#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_InvMain.generated.h"

class UWidgetSwitcher;
class UUK_InvTapbutton;
class UUK_InvUI;
class UUK_InvInfo;
class UDataTable;
class UUK_MoneyWidget;

UENUM(BlueprintType)
enum class EMainTab : uint8
{
	Inventory UMETA(DisplayName = "Inventory"),
	System UMETA(DisplayName = "System"),
	Map UMETA(DisplayName = "Map")
};

UCLASS()
class UK_API UUK_InvMain : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* InvSwitcher;

	UPROPERTY(meta = (BindWidget))
	UUK_InvTapbutton* TapSystem;

	UPROPERTY(meta = (BindWidget))
	UUK_InvTapbutton* TapInventory;

	UPROPERTY(meta = (BindWidget))
	UUK_InvTapbutton* TapMap;

	UPROPERTY(meta = (BindWidget))
	UUK_InvUI* InvUI;

	UPROPERTY(meta = (BindWidget))
	UUK_InvInfo* InvInfo;

	UPROPERTY(meta = (BindWidgetOptional))
	UUK_MoneyWidget* MoneyWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inv")
	TArray<TObjectPtr<UDataTable>> ItemDataTables;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void TapClicked(UUK_InvTapbutton* ClickTap);

	UFUNCTION()
	void OnPreviewSlot(const FInventorySlot& SlotData);

	UFUNCTION()
	void OnPreviewCleared();

	UFUNCTION(BlueprintCallable)
	void SetMainTab(EMainTab NewTab);
};