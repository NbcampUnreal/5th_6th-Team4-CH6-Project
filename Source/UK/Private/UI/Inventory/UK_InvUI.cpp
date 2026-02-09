#include "UI/Inventory/UK_InvUI.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Inventory/UK_CategoryTap.h"

void UUK_InvUI::NativeConstruct()
{
	Super::NativeConstruct();

	if(TapALL)
	{
		TapALL->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if(TapWeapon)
	{
		TapWeapon->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if(TapFood)
	{
		TapFood->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if(TapMaterial)
	{
		TapMaterial->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}
}

void UUK_InvUI::CategoryTap(UUK_CategoryTap* CategoryTap)
{
	if(!InvCateSwitcher ) return;

	if(CategoryTap == TapALL)
	{
		InvCateSwitcher->SetActiveWidgetIndex(0);
	}
	else if(CategoryTap == TapWeapon)
	{
		InvCateSwitcher->SetActiveWidgetIndex(1);
	}
	else if(CategoryTap == TapFood)
	{
		InvCateSwitcher->SetActiveWidgetIndex(2);
	}
	else if(CategoryTap == TapMaterial)
	{
		InvCateSwitcher->SetActiveWidgetIndex(3);
	}
}
