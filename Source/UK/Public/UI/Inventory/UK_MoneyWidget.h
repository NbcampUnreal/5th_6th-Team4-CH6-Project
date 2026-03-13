#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_MoneyWidget.generated.h"

class UTextBlock;
class UUK_InventoryComponent;

UCLASS()
class UK_API UUK_MoneyWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoneyText;

	UFUNCTION(BlueprintCallable)
	void BindInventoryComponent(UUK_InventoryComponent* InInvComp);

	UFUNCTION()
	void UpdateMoney(int32 NewGold);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY()
	UUK_InventoryComponent* InvComp = nullptr;
	
};
