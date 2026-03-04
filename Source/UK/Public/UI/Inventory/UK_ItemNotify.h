#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_ItemNotify.generated.h"

class UTextBlock;
class UImage;
class UDataTable;

UCLASS()
class UK_API UUK_ItemNotify : public UUserWidget
{
	GENERATED_BODY()
	
public:

	//바인드
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemAmount;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemNotify")
	UDataTable* ItemDataTable;

	UFUNCTION(BlueprintCallable)
	void NotifyItem(FName ItemID, int32 Amount);

protected:

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* SlideOpenAnimation;


};
