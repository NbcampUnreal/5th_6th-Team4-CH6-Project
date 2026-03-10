#include "UI/Inventory/UK_CategoryTap.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Styling/SlateTypes.h"

void UUK_CategoryTap::NativeConstruct()
{
	Super::NativeConstruct();
	if (CategoryButton)
	{
		CategoryButton->OnClicked.AddDynamic(this, &UUK_CategoryTap::CategoryTapClicked);

		CategoryButton->SetStyle(CustomButtonStyle);
	}
}

void UUK_CategoryTap::CategoryTapClicked()
{
	OnCategoryTap.Broadcast(this);
}

void UUK_CategoryTap::SetSelected(bool bInSelected)
{
	bSelected = bInSelected;

	if ( !CategoryButton ) return;

	FButtonStyle NewStyle = CustomButtonStyle;

	if ( bSelected )
	{

		NewStyle.SetNormal(CustomButtonStyle.Pressed);
		NewStyle.SetHovered(CustomButtonStyle.Pressed);
	}
	else
	{
		NewStyle = CustomButtonStyle;
	}

	CategoryButton->SetStyle(NewStyle);
}
