#include "UI/Inventory/UK_InvMain.h"
#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/WidgetSwitcher.h"

void UUK_InvMain::NativeConstruct()
{
	Super::NativeConstruct();

	//스위치어 버튼 바인드
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
	//탭 버튼에 따른 위젯 스위처 인덱스로 변경, 위젯에서 인덱스에 맞게 표시해야함
	if(ClickTap == TapInventory)
	{
		InvSwitcher->SetActiveWidgetIndex(0);
	}
	else if(ClickTap == TapSystem)
	{
		InvSwitcher->SetActiveWidgetIndex(1);
	}
	else if(ClickTap == TapMap)
	{
		InvSwitcher->SetActiveWidgetIndex(2);
	}
}
