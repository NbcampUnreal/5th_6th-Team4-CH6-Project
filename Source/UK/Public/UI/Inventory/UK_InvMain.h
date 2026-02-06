#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_InvMain.generated.h"

class UWidgetSwitcher;
class UUK_InvTapbutton;

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



	virtual void NativeConstruct() override;

	UFUNCTION()
	void TapClicked(UUK_InvTapbutton* ClickTap);
	
};
