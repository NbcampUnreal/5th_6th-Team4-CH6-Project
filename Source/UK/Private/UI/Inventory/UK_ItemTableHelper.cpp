#include "UI/Inventory/UK_ItemTableHelper.h"
#include "Engine/DataTable.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "DataAsset/Data/UK_WeaponItemData.h"

bool UK_ItemTableHelper::FindItemData(const TArray<TObjectPtr<UDataTable>>& Tables, FName ItemID, FUK_ItemTableRowView& OutRow)
{
	OutRow = FUK_ItemTableRowView();

	if (ItemID.IsNone())
	{
		return false;
	}

	for (UDataTable* Table : Tables)
	{
		if (!IsValid(Table)) continue;

		const UScriptStruct* RowStruct = Table->GetRowStruct();
		if (!RowStruct)	continue;
		

		if (RowStruct == FUK_ItemData::StaticStruct())
		{
			if (const FUK_ItemData* Row = Table->FindRow<FUK_ItemData>(ItemID, TEXT("UK_ItemTableHelper::FindItemData")))
			{
				OutRow.ItemID = Row->ItemID;
				OutRow.ItemName = Row->ItemName;
				OutRow.ItemDescription = Row->ItemDescription;
				OutRow.ItemIcon = Row->ItemIcon;
				OutRow.ItemStackCount = Row->ItemStackCount;
				OutRow.MaxItemStack = Row->MaxItemStack;
				OutRow.ItemTag = Row->ItemTag;
				OutRow.bIsWeapon = false;
				return true;
			}
		}
		else if (RowStruct == FUK_WeaponItemData::StaticStruct())
		{
			if (const FUK_WeaponItemData* Row = Table->FindRow<FUK_WeaponItemData>(ItemID, TEXT("UK_ItemTableHelper::FindItemData")))
			{
				OutRow.ItemID = Row->ItemID;
				OutRow.ItemName = Row->ItemName;
				OutRow.ItemDescription = Row->ItemDescription;
				OutRow.ItemIcon = Row->ItemIcon;
				OutRow.ItemStackCount = Row->ItemStackCount;
				OutRow.MaxItemStack = Row->MaxItemStack;
				OutRow.ItemTag = Row->ItemTag;
				OutRow.bIsWeapon = true;
				OutRow.WeaponAttribute = Row->WeaponAttribute;
				return true;
			}
		}
	}

	return false;
}