#include "UI/Inventory/UK_InvTapbutton.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

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
	}
}

void UUK_InvTapbutton::ButtonTapClicked()
{
	OnButtonTap.Broadcast(this);
}
