#include "UI/Inventory/UK_InvCategoryBase.h"
#include "Components/UniformGridPanel.h"

//Slot
#include "UI/Inventory/UK_InvSlot.h"
//GamePlayTags
#include "Tags/UK_GameplayTags.h"
//ItemData
#include "DataAsset/Data/UK_ItemData.h"

void UUK_InvCategoryBase::NativeConstruct()
{
	Super::NativeConstruct();

	WeaponRootTag = UK_GameplayTags::Weapon::WeaponRoot;
	CreateSlots();
}

void UUK_InvCategoryBase::CreateSlots()
{
	if ( !SlotGrid || !InvSlotClass )
	{
		return;
	}

	SlotGrid->ClearChildren();
	SlotWidgets.Empty();

	// 현재 슬롯 수는 최대 슬롯을 넘지 못하게 제한
	const int32 SlotCountToCreate = FMath::Clamp(CurrentSlot, 0, MaxSlot);

	for ( int32 i = 0; i < SlotCountToCreate; ++i )
	{
		UUK_InvSlot* SlotWidget = CreateWidget<UUK_InvSlot>(this, InvSlotClass);
		if ( !SlotWidget ) continue;

		SlotWidget->SlotIndex = i;
		SlotWidget->ItemDataTable = ItemDataTable;

		SlotWidget->bAllowDrag = bAllowSlotDrag;
		SlotWidget->WeaponRootTag = WeaponRootTag;

		SlotWidget->OnSlotHovered.AddDynamic(this, &UUK_InvCategoryBase::HandleSlotHovered);
		SlotWidget->OnSlotUnhovered.AddDynamic(this, &UUK_InvCategoryBase::HandleSlotUnhovered);

		SlotGrid->AddChildToUniformGrid(
			SlotWidget,
			i / SlotColumns,
			i % SlotColumns
		);

		SlotWidgets.Add(SlotWidget);
	}

}

void UUK_InvCategoryBase::SetInvArraySlots(const TArray<FInventorySlot>& InAllSlots)
{
	//invUI -> 카테고리 -> 슬롯 위젯, 빈 슬롯은 표시X
	FilteredSlots.Empty();

	for ( const FInventorySlot& InvSlot : InAllSlots )
	{
		//빈 슬롯은 건너뜀
		if (InvSlot.isEmpty())
			continue;
		//카테고리에 맞지 않는 슬롯은 건너뜀
		if (!IsItemAllowed(InvSlot)) 
			continue;
		//필터링된 슬롯에 추가
		FilteredSlots.Add(InvSlot);

		// UI에 표시 가능한 슬롯 수까지만
		if (FilteredSlots.Num() >= CurrentSlot) 
			break;
	}

	UpdateSlots();
}

void UUK_InvCategoryBase::UpdateSlots()
{
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (!SlotWidgets[i]) continue;

		if (FilteredSlots.IsValidIndex(i))
		{
			SlotWidgets[i]->SlotData = FilteredSlots[i];
		}
		else
		{
			SlotWidgets[i]->SlotData.Clear();
		}
		SlotWidgets[i]->UpdateSlot();
	}
}

void UUK_InvCategoryBase::HandleSlotHovered(const FInventorySlot& SlotData)
{
	OnCategorySlotHovered.Broadcast(SlotData);
}

void UUK_InvCategoryBase::HandleSlotUnhovered()
{
	OnCategorySlotUnhovered.Broadcast();
}

void UUK_InvCategoryBase::AddSlot(int32 AddCount)
{
	if ( AddCount <= 0 )
		return;

	CurrentSlot = FMath::Clamp(
		CurrentSlot + AddCount,
		0,
		MaxSlot
	);

	CreateSlots();
	UpdateSlots();
}

bool UUK_InvCategoryBase::IsItemAllowed(const FInventorySlot& InSlot) const
{
	if (CategoryType == EInvCategory::All)
	{
		return true;
	}

	if (!ItemDataTable)
	{
		return false;
	}

	const FUK_ItemData* ItemData = ItemDataTable->FindRow<FUK_ItemData>(InSlot.ItemID, TEXT("IsItemAllowed"));

	if (!ItemData)
	{
		return false;
	}

	const FGameplayTag& ItemTag = ItemData->ItemTag;

	switch (CategoryType)
	{
	case EInvCategory::Weapon:
		return ItemTag.MatchesTag(UK_GameplayTags::Weapon::WeaponRoot);

	case EInvCategory::Food:
		return ItemTag.MatchesTag(UK_GameplayTags::Food::FoodRoot);

	case EInvCategory::Material:
		return ItemTag.MatchesTag(UK_GameplayTags::Material::MaterialRoot);

	default:
		return true;
	}
}