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

UCLASS()
class UK_API UUK_InvMain : public UUserWidget
{
	GENERATED_BODY()

public:

	//바인드
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* InvSwitcher;

	UPROPERTY(meta = ( BindWidget ))
	UUK_InvTapbutton* TapSystem;

	UPROPERTY(meta = ( BindWidget ))
	UUK_InvTapbutton* TapInventory;

	UPROPERTY(meta = ( BindWidget ))
	UUK_InvTapbutton* TapMap;


	UPROPERTY(meta = (BindWidget))
	UUK_InvUI* InvUI;

	UPROPERTY(meta = (BindWidget))
	UUK_InvInfo* InvInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inv")
	UDataTable* ItemDataTable;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void TapClicked(UUK_InvTapbutton* ClickTap);

	//info
	UFUNCTION()
	void OnPreviewSlot(const FInventorySlot& SlotData);
	UFUNCTION()
	void OnPreviewCleared();
	
};
