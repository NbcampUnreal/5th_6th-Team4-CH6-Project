#include "UI/Inventory/UK_InvMain.h"

//UI
#include "UI/Inventory/UK_InvInfo.h"
#include "UI/Inventory/UK_InvUI.h"
#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/WidgetSwitcher.h"

void UUK_InvMain::NativeConstruct()
{
	Super::NativeConstruct();

	if (InvInfo)
	{
		InvInfo->SetVisibility(ESlateVisibility::Hidden);
	}

	if (InvUI)
	{
		InvUI->OnInvSlotPreview.AddDynamic(this, &UUK_InvMain::OnPreviewSlot);
		InvUI->OnInvSlotPreviewCleared.AddDynamic(this, &UUK_InvMain::OnPreviewCleared);
	}
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

void UUK_InvMain::OnPreviewSlot(const FInventorySlot& SlotData)
{
	if (!InvInfo) return;
	InvInfo->SlotMouse(ItemDataTable, SlotData, 24.f, 24.f);
}

void UUK_InvMain::OnPreviewCleared()
{
	if (!InvInfo) return;
	InvInfo->HideToolInfo();
}