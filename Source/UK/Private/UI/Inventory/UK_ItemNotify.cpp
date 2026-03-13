#include "UI/Inventory/UK_ItemNotify.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "UI/Inventory/UK_ItemTableHelper.h"

void UUK_ItemNotify::NotifyItem(FName ItemID, int32 Amount)
{
	if (ItemDataTables.Num() == 0 || !ItemName || !ItemAmount) return;

	FUK_ItemTableRowView Row;

	if ( !UK_ItemTableHelper::FindItemData(ItemDataTables, ItemID, Row) ) return;

	ItemName->SetText(Row.ItemName);
	ItemAmount->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Amount)));

	if (ItemIcon)
	{
		if (UTexture2D* Tex = Row.ItemIcon.LoadSynchronous())
		{
			ItemIcon->SetBrushFromTexture(Tex, true);
		}
		else
		{
			ItemIcon->SetBrushFromTexture(nullptr, true);
		}
	}

	CachedItemID = ItemID;
	CachedAmount = Amount;
	UpdateAmountText();

	if (SlideOpenAnimation)
	{
		UnbindAllFromAnimationFinished(SlideOpenAnimation);
		PlayAnimation(SlideOpenAnimation);
	}

	StartAutoRemove();

}

void UUK_ItemNotify::StartAutoRemove()
{
	if (!GetWorld()) return;

	auto& Timer = GetWorld()->GetTimerManager();
	Timer.ClearTimer(AutoRemoveTimer);
	Timer.SetTimer(AutoRemoveTimer, this, &UUK_ItemNotify::PlayCloseAnimation, AutoRemoveDelay, false);
}

void UUK_ItemNotify::PlayCloseAnimation()
{
	if (SlideOpenAnimation)
	{
		bClosing = true;
		UnbindAllFromAnimationFinished(SlideOpenAnimation);

		FWidgetAnimationDynamicEvent FinishedEvent;
		FinishedEvent.BindDynamic(this, &UUK_ItemNotify::OnCloseAnimFinished);
		BindToAnimationFinished(SlideOpenAnimation, FinishedEvent);
		PlayAnimationReverse(SlideOpenAnimation);
	}
	else
	{
		RemoveFromParent();
	}
}

void UUK_ItemNotify::OnCloseAnimFinished()
{
	if ( !bClosing ) return;
	bClosing = false;

	if (SlideOpenAnimation)
	{
		UnbindAllFromAnimationFinished(SlideOpenAnimation);
	}

	RemoveFromParent();
}

void UUK_ItemNotify::AddAmount(int32 DeltaAmount)
{
	CachedAmount += DeltaAmount;
	if ( CachedAmount < 0 ) CachedAmount = 0;

	UpdateAmountText();

	bClosing = false;
	if (SlideOpenAnimation)
	{
		UnbindAllFromAnimationFinished(SlideOpenAnimation);
		PlayAnimation(SlideOpenAnimation, 0.f);
	}

	StartAutoRemove();
}

void UUK_ItemNotify::UpdateAmountText()
{
	if (ItemAmount)
	{
		ItemAmount->SetText(FText::FromString(FString::Printf(TEXT("x%d"), CachedAmount)));
	}
}