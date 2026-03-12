#include "UI/Inventory/UK_InvCategoryBase.h"
#include "Components/UniformGridPanel.h"
#include "UI/Inventory/UK_InvSlot.h"
#include "Tags/UK_GameplayTags.h"
#include "UI/Inventory/UK_ItemTableHelper.h"

void UUK_InvCategoryBase::NativeConstruct()
{
	Super::NativeConstruct();

	WeaponRootTag = UK_GameplayTags::Weapon::WeaponRoot;
	CreateSlots();
}

void UUK_InvCategoryBase::CreateSlots()
{
	if (!SlotGrid || !InvSlotClass)
	{
		return;
	}

	SlotGrid->ClearChildren();
	SlotWidgets.Empty();

	const int32 SlotCountToCreate = FMath::Clamp(CurrentSlot, 0, MaxSlot);

	for (int32 i = 0; i < SlotCountToCreate; ++i)
	{
		UUK_InvSlot* SlotWidget = CreateWidget<UUK_InvSlot>(this, InvSlotClass);
		if (!SlotWidget) continue;

		SlotWidget->SlotIndex = i;
		SlotWidget->ItemDataTables = ItemDataTables;
		SlotWidget->bAllowDrag = bAllowSlotDrag;
		SlotWidget->WeaponRootTag = WeaponRootTag;

		SlotWidget->OnSlotHovered.AddDynamic(this, &UUK_InvCategoryBase::HandleSlotHovered);
		SlotWidget->OnSlotUnhovered.AddDynamic(this, &UUK_InvCategoryBase::HandleSlotUnhovered);

		SlotGrid->AddChildToUniformGrid(SlotWidget,i / SlotColumns,i % SlotColumns);
		SlotWidgets.Add(SlotWidget);
	}
}

void UUK_InvCategoryBase::SetInvArraySlots(const TArray<FInventorySlot>& InAllSlots)
{
	FilteredSlots.Empty();

	for (const FInventorySlot& InvSlot : InAllSlots)
	{
		if (InvSlot.isEmpty())
			continue;

		if (!IsItemAllowed(InvSlot))
			continue;

		FilteredSlots.Add(InvSlot);

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
	if (AddCount <= 0)
		return;

	CurrentSlot = FMath::Clamp(CurrentSlot + AddCount, 0, MaxSlot);

	CreateSlots();
	UpdateSlots();
}

bool UUK_InvCategoryBase::IsItemAllowed(const FInventorySlot& InSlot) const
{
	if (CategoryType == EInvCategory::All )	return true;
	
	FUK_ItemTableRowView ItemInfo;
	if (!UK_ItemTableHelper::FindItemData(ItemDataTables, InSlot.ItemID, ItemInfo))	return false;	

	const FGameplayTag& ItemTag = ItemInfo.ItemTag;

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