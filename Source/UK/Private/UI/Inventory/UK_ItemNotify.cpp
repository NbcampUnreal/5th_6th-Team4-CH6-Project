#include "UI/Inventory/UK_ItemNotify.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"

#include "DataAsset/Data/UK_ItemData.h"


void UUK_ItemNotify::NotifyItem(FName ItemID, int32 Amount)
{
	if (!ItemDataTable || !ItemName || !ItemAmount) return;

	const FUK_ItemData* Row = ItemDataTable->FindRow<FUK_ItemData>(ItemID, TEXT("ItemNotify"));
	if (!Row) return;


	ItemName->SetText(Row->ItemName);

	if (ItemIcon)
	{
		if ( UTexture2D* Tex = Row->ItemIcon.LoadSynchronous())
		{
			ItemIcon->SetBrushFromTexture(Tex, true);
		}
	}

	if (SlideOpenAnimation)
	{
		PlayAnimation(SlideOpenAnimation);
	}
}
