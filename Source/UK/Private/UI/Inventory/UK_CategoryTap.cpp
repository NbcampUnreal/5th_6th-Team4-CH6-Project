#include "UI/Inventory/UK_CategoryTap.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UUK_CategoryTap::NativeConstruct()
{
	Super::NativeConstruct();
	if (CategoryButton)
	{
		CategoryButton->OnClicked.AddDynamic(this, &UUK_CategoryTap::CategoryTapClicked);
	}
}

void UUK_CategoryTap::CategoryTapClicked()
{
	OnCategoryTap.Broadcast(this);
}
