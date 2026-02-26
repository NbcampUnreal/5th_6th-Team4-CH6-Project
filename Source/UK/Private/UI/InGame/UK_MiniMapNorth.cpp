#include "UI/InGame/UK_MiniMapNorth.h"
#include "Components/TextBlock.h"

void UUK_MiniMapNorth::SetDirection(const FString& InText)
{
	if (TextLabel)
	{
		TextLabel->SetText(FText::FromString(InText));
	}
}
