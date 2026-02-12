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

	//처음에는 안보이게, 툴팁이 슬롯을 막을 경우가 있어서
	SetVisibility(ESlateVisibility::Hidden);
	SetIsEnabled(false);
}
//슬롯에 마우스 올렸을 때 정보 보여주기
void UUK_InvInfo::SlotMouse(UDataTable* InItemDataTable, const FInventorySlot& InSlot, float OffsetX, float OffsetY)
{
	ItemDataTable = InItemDataTable;

	if(!ItemDataTable || InSlot.isEmpty())
	{
		HideToolInfo();
		return;
	}

	ApplyItemID(InSlot.ItemID);

	FVector2D MousePos;

	const bool bIsMouse = UWidgetLayoutLibrary::GetMousePositionScaledByDPI(GetOwningPlayer(), MousePos.X, MousePos.Y);

	if(!bIsMouse)
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

	if (ItemImage)
	{
		ItemImage->SetBrushFromTexture(nullptr);
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
	}
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

	if (ItemImage)
	{
		UTexture2D* Icon = ItemInfo->ItemIcon.LoadSynchronous();
		ItemImage->SetBrushFromTexture(Icon);
		ItemImage->SetVisibility(Icon ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

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

//화면 밖으로 나가지 않도록 위치 설정
void UUK_InvInfo::SetTooltipPositionClamped(const FVector2D& MousePos, const FVector2D& Offset)
{
	//const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);

	//ForceLayoutPrepass();
	//const FVector2D TooltipSize = GetDesiredSize();

	//FVector2D Pos = MousePos + Offset;

	////우측/하단 초과하면 반대로 띄우기
	//if (Pos.X + TooltipSize.X > ViewportSize.X)
	//	Pos.X = MousePos.X - Offset.X - TooltipSize.X;

	//if (Pos.Y + TooltipSize.Y > ViewportSize.Y)
	//	Pos.Y = MousePos.Y - Offset.Y - TooltipSize.Y;

	////최소 0 보정
	//Pos.X = FMath::Max(0.f, Pos.X);
	//Pos.Y = FMath::Max(0.f, Pos.Y);

	//SetPositionInViewport(Pos, true);
}
