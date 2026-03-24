#include "UI/Inventory/UK_CategoryTap.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Styling/SlateTypes.h"

void UUK_CategoryTap::NativeConstruct()
{
	Super::NativeConstruct();
	if (CategoryButton)
	{
		CategoryButton->OnClicked.AddDynamic(this, &UUK_CategoryTap::CategoryTapClicked);
		CategoryButton->OnHovered.AddDynamic(this, &UUK_CategoryTap::OnHovered);
		CategoryButton->OnUnhovered.AddDynamic(this, &UUK_CategoryTap::OnUnHovered);
		
		CategoryButton->SetStyle(CustomButtonStyle);
	}
}

void UUK_CategoryTap::CategoryTapClicked()
{
	OnCategoryTap.Broadcast(this);
	
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
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

void UUK_CategoryTap::OnHovered()
{
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound);
	}
}

void UUK_CategoryTap::OnUnHovered()
{
	if (UnHoverSound)
	{
		UGameplayStatics::PlaySound2D(this, UnHoverSound);
	}
}
