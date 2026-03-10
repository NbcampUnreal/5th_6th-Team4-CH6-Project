#include "UI/Inventory/UK_DraggedItem.h"
#include "Components/Image.h"

void UUK_DraggedItem::SetIcon(UTexture2D* IconTexture)
{
	if ( ItemIcon && IconTexture )
	{
		ItemIcon->SetBrushFromTexture(IconTexture);
	}
}
