#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_InvUI.generated.h"

class UUK_CategoryTap;
class UWidgetSwitcher;
class UUK_InventoryComponent;
class UUK_InvCategoryBase;

UCLASS()
class UK_API UUK_InvUI : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	void BindInventoryComponent(UUK_InventoryComponent* InInvComp);

	UFUNCTION()
	void OnInvCompUpdated();

	//스위쳐 바인드
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* InvCateSwitcher;

	UPROPERTY(meta = (BindWidget))
	UUK_CategoryTap* TapALL;

	UPROPERTY(meta = (BindWidget))
	UUK_CategoryTap* TapWeapon;

	UPROPERTY(meta = (BindWidget))
	UUK_CategoryTap* TapFood;

	UPROPERTY(meta = (BindWidget))
	UUK_CategoryTap* TapMaterial;

	//카데고리 베이스 바인드
	UPROPERTY(meta = (BindWidget))
	UUK_InvCategoryBase* CategoryALL;

	UPROPERTY(meta = (BindWidget))
	UUK_InvCategoryBase* CategoryWeapon;

	UPROPERTY(meta = (BindWidget))
	UUK_InvCategoryBase* CategoryFood;

	UPROPERTY(meta = (BindWidget))
	UUK_InvCategoryBase* CategoryMaterial;

	UPROPERTY()
	UUK_InventoryComponent* InvComp;

	UFUNCTION()
	void CategoryTap(UUK_CategoryTap* CategoryTap);
	
};
