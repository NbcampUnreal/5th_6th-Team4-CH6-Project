#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void UUK_InvTapbutton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ButtonText)
	{
		ButtonText->SetText(InText);
	}
}

void UUK_InvTapbutton::NativeConstruct()
{
	Super::NativeConstruct();
	if (ButtonTap)
	{
		ButtonTap->OnClicked.AddDynamic(this, &UUK_InvTapbutton::ButtonTapClicked);
		ButtonTap->OnHovered.AddDynamic(this, &UUK_InvTapbutton::OnHovered);
		ButtonTap->OnUnhovered.AddDynamic(this, &UUK_InvTapbutton::OnUnHovered);
	}
}

void UUK_InvTapbutton::ButtonTapClicked()
{
	OnButtonTap.Broadcast(this);
	
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}

void UUK_InvTapbutton::OnHovered()
{
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound);
	}
}

void UUK_InvTapbutton::OnUnHovered()
{
	if (UnHoverSound)
	{
		UGameplayStatics::PlaySound2D(this, UnHoverSound);
	}
}
