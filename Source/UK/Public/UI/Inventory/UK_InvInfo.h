// Fill out your copyright notice in the Description page of Project Settings.

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
	//DT는 InvMain에서 넣어줄 예정 (또는 BP에 고정해도 됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo")
	UDataTable* ItemDataTable = nullptr;

	//바인드
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UTextBlock* ItemNameText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UTextBlock* ItemDescriptionText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UImage* ItemImage = nullptr; 

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UImage* ItemTypeImage = nullptr;

	//타입 아이콘들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo|TypeIcon")
	UTexture2D* WeaponTypeIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo|TypeIcon")
	UTexture2D* FoodTypeIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvInfo|TypeIcon")
	UTexture2D* MaterialTypeIcon = nullptr;

	UFUNCTION(BlueprintCallable, Category = "InvInfo")
	void SlotMouse(UDataTable* InItemDataTable, const FInventorySlot& InSlot, float OffsetX = 24.f, float OffsetY = 24.f);

	UFUNCTION(BlueprintCallable, Category = "InvInfo")
	void HideToolInfo();

	UFUNCTION(BlueprintCallable, Category = "InvInfo")
	void ClearInfo();

	virtual void NativeConstruct() override;

	void ApplyItemID(FName ItemID);
	void ApplyItemTag(const struct FGameplayTag& Tag);
	void SetTooltipPositionClamped(const FVector2D& MousePos, const FVector2D& Offset);
};
