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

	FTimerHandle AutoRemoveTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemNotify")
	float AutoRemoveDelay = 2.0f;

	void StartAutoRemove();
	void PlayCloseAnimation(); 

	UFUNCTION()                  
	void OnCloseAnimFinished();
	bool bClosing = false;

	void AddAmount(int32 DeltaAmount);
	void UpdateAmountText();

	FName GetItemID() const {return CachedItemID;} 
	FName CachedItemID;      
	int32 CachedAmount = 0; 


protected:

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* SlideOpenAnimation;
 
};
