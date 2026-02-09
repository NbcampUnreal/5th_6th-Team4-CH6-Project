#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_InvUI.generated.h"

class UUK_CategoryTap;
class UWidgetSwitcher;


UCLASS()
class UK_API UUK_InvUI : public UUserWidget
{
	GENERATED_BODY()

public:

	//바인드
	UPROPERTY(meta = ( BindWidget ))
	UWidgetSwitcher* InvCateSwitcher;

	UPROPERTY(meta = ( BindWidget ))
	UUK_CategoryTap* TapALL;

	UPROPERTY(meta = ( BindWidget ))
	UUK_CategoryTap* TapWeapon;

	UPROPERTY(meta = ( BindWidget ))
	UUK_CategoryTap* TapFood;

	UPROPERTY(meta = ( BindWidget ))
	UUK_CategoryTap* TapMaterial;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void CategoryTap(UUK_CategoryTap* CategoryTap);
	
};
