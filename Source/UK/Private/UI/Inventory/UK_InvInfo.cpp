#include "UI/Inventory/UK_InvInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/Inventory/UK_ItemTableHelper.h"
#include "Tags/UK_GameplayTags.h"
#include "GameplayTagContainer.h"

void UUK_InvInfo::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
	SetIsEnabled(true);
}

void UUK_InvInfo::SlotMouse(const TArray<UDataTable*>& InItemDataTables, const FInventorySlot& InSlot, float OffsetX, float OffsetY)
{
	ItemDataTables.Reset();

	for ( UDataTable* Table : InItemDataTables )
	{
		ItemDataTables.Add(Table);
	}

	if ( ItemDataTables.Num() == 0 || InSlot.isEmpty() )
	{
		HideToolInfo();
		return;
	}

	FVector2D MousePos;
	ApplyItemID(InSlot.ItemID);

	const bool bIsMouse = UWidgetLayoutLibrary::GetMousePositionScaledByDPI(GetOwningPlayer(), MousePos.X, MousePos.Y);

	if ( !bIsMouse )
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UUK_InvInfo::HideToolInfo()
{
	ClearInfo();
	SetVisibility(ESlateVisibility::Hidden);
}

void UUK_InvInfo::ClearInfo()
{
	if ( ItemNameText ) ItemNameText->SetText(FText::GetEmpty());
	if ( ItemDescriptionText ) ItemDescriptionText->SetText(FText::GetEmpty());

	if ( ItemTypeImage )
	{
		ItemTypeImage->SetBrushFromTexture(nullptr);
		ItemTypeImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUK_InvInfo::ApplyItemID(FName ItemID)
{
	FUK_ItemTableRowView ItemInfo;
	if ( !UK_ItemTableHelper::FindItemData(ItemDataTables, ItemID, ItemInfo) )
	{
		ClearInfo();
		return;
	}

	if ( ItemNameText ) ItemNameText->SetText(ItemInfo.ItemName);
	if ( ItemDescriptionText ) ItemDescriptionText->SetText(ItemInfo.ItemDescription);

	ApplyItemTag(ItemInfo.ItemTag);
}

void UUK_InvInfo::ApplyItemTag(const FGameplayTag& Tag)
{
	if ( !ItemTypeImage ) return;

	UTexture2D* TypeIcon = nullptr;

	if ( Tag.IsValid() )
	{
		if ( Tag.MatchesTag(UK_GameplayTags::Weapon::WeaponRoot) )
		{
			TypeIcon = WeaponTypeIcon;
		}
		else if ( Tag.MatchesTag(UK_GameplayTags::Food::FoodRoot) )
		{
			TypeIcon = FoodTypeIcon;
		}
		else if ( Tag.MatchesTag(UK_GameplayTags::Material::MaterialRoot) )
		{
			TypeIcon = MaterialTypeIcon;
		}
	}

	if ( TypeIcon )
	{
		ItemTypeImage->SetBrushFromTexture(TypeIcon);
		ItemTypeImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemTypeImage->SetBrushFromTexture(nullptr);
		ItemTypeImage->SetVisibility(ESlateVisibility::Hidden);
	}
}