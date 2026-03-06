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
	ItemAmount->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Amount)));

	if (ItemIcon)
	{
		if (UTexture2D* Tex = Row->ItemIcon.LoadSynchronous())
		{
			ItemIcon->SetBrushFromTexture(Tex, true);
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
	if (!bClosing) return;
	bClosing = false;

	if (SlideOpenAnimation)
	{
		UnbindAllFromAnimationFinished(SlideOpenAnimation);
	}

	RemoveFromParent();
}

void UUK_ItemNotify::AddAmount(int32 DeltaAmount)
{
	//현재 수량에 델타를 더하고, 0보다 작아지면 0으로 고정
	CachedAmount += DeltaAmount;
	if (CachedAmount < 0) CachedAmount = 0;

	UpdateAmountText();

	//애니메이션이 재생 중이 아니라면 다시 재생
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
