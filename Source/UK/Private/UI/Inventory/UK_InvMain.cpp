#include "UI/Inventory/UK_InvMain.h"
#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/WidgetSwitcher.h"

void UUK_InvMain::NativeConstruct()
{
	Super::NativeConstruct();

	if(TapSystem)
	{
		TapSystem->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if(TapInventory)
	{
		TapInventory->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}

	if(TapMap)
	{
		TapMap->OnButtonTap.AddDynamic(this, &UUK_InvMain::TapClicked);
	}
}

void UUK_InvMain::TapClicked(UUK_InvTapbutton* ClickTap)
{
	if(!InvSwitcher) return;

	if(ClickTap == TapInventory)
	{
		InvSwitcher->SetActiveWidgetIndex(0);
	}
	else if(ClickTap == TapSystem)
	{
		InvSwitcher->SetActiveWidgetIndex(1);
	}
	else if ( ClickTap == TapMap )
	{
		InvSwitcher->SetActiveWidgetIndex(2);
	}
}
