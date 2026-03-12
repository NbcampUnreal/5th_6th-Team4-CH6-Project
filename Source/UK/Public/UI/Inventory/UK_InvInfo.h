#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_InvInfo.generated.h"

class UTextBlock;
class UDataTable;
class UImage;
class UTexture2D;

UCLASS()
class UK_API UUK_InvInfo : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo")
	TArray<TObjectPtr<UDataTable>> ItemDataTables;

	UPROPERTY(BlueprintReadWrite, meta = ( BindWidgetOptional ))
	UTextBlock* ItemNameText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = ( BindWidgetOptional ))
	UTextBlock* ItemDescriptionText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = ( BindWidgetOptional ))
	UImage* ItemTypeImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo|TypeIcon")
	UTexture2D* WeaponTypeIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo|TypeIcon")
	UTexture2D* FoodTypeIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo|TypeIcon")
	UTexture2D* MaterialTypeIcon = nullptr;

	UFUNCTION(BlueprintCallable, Category = "InvInfo")
	void SlotMouse(const TArray<UDataTable*>& InItemDataTables, const FInventorySlot& InSlot, float OffsetX = 24.f, float OffsetY = 24.f);

	UFUNCTION(BlueprintCallable, Category = "InvInfo")
	void HideToolInfo();

	UFUNCTION(BlueprintCallable, Category = "InvInfo")
	void ClearInfo();

	virtual void NativeConstruct() override;
	void ApplyItemID(FName ItemID);
	void ApplyItemTag(const struct FGameplayTag& Tag);
};