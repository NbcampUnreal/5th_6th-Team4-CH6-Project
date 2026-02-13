#include "UI/Inventory/UK_InvInfo.h"

//UI
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
//Data
#include "DataAsset/Data/UK_ItemData.h"
#include "Tags/UK_GameplayTags.h"
#include "GameplayTagContainer.h"

void UUK_InvInfo::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
	SetIsEnabled(true);
}
//슬롯에 마우스 올렸을 때 정보 보여주기
void UUK_InvInfo::SlotMouse(UDataTable* InItemDataTable, const FInventorySlot& InSlot, float OffsetX, float OffsetY)
{
	if(!ItemDataTable || InSlot.isEmpty())
	{
		HideToolInfo();
		return;
	}

	FVector2D MousePos;
	ApplyItemID(InSlot.ItemID);

	const bool bIsMouse = UWidgetLayoutLibrary::GetMousePositionScaledByDPI(GetOwningPlayer(), MousePos.X, MousePos.Y);

	if (!bIsMouse)
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetTooltipPositionClamped(MousePos, FVector2D(OffsetX, OffsetY));
}

//정보 숨기기
void UUK_InvInfo::HideToolInfo()
{ 
	ClearInfo();
	SetVisibility(ESlateVisibility::Hidden);
}
//정보 초기화
void UUK_InvInfo::ClearInfo()
{
	if (ItemNameText)ItemNameText->SetText(FText::GetEmpty());
	if (ItemDescriptionText) ItemDescriptionText->SetText(FText::GetEmpty());

	if (ItemTypeImage)
	{
		ItemTypeImage->SetBrushFromTexture(nullptr);
		ItemTypeImage->SetVisibility(ESlateVisibility::Hidden);
	
	}
}
//아이템 ID로 정보 적용
void UUK_InvInfo::ApplyItemID(FName ItemID)
{
	const FUK_ItemData* ItemInfo = ItemDataTable ? ItemDataTable->FindRow<FUK_ItemData>(ItemID, TEXT("InvInfo")) : nullptr;
	if (!ItemInfo)
	{
		ClearInfo();
		return;
	}

	if (ItemNameText) ItemNameText->SetText(ItemInfo->ItemName);
	if (ItemDescriptionText) ItemDescriptionText->SetText(ItemInfo->ItemDescription);

	ApplyItemTag(ItemInfo->ItemTag);
}

void UUK_InvInfo::ApplyItemTag(const FGameplayTag& Tag)
{
	if (!ItemTypeImage) return;

	UTexture2D* TypeIcon = nullptr;

	if (Tag.IsValid())
	{
		if (Tag.MatchesTag(UK_GameplayTags::Weapon::WeaponRoot))
		{
			TypeIcon = WeaponTypeIcon;
		}
		else if (Tag.MatchesTag(UK_GameplayTags::Food::FoodRoot))
		{
			TypeIcon = FoodTypeIcon;
		}
		else if (Tag.MatchesTag(UK_GameplayTags::Material::MaterialRoot))
		{
			TypeIcon = MaterialTypeIcon;
		}
	}

	if (TypeIcon)
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

void UUK_InvInfo::SetTooltipPositionClamped(const FVector2D& MousePos, const FVector2D& Offset)
{
}