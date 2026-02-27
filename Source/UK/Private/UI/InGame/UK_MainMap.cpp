#include "UI/InGame/UK_MainMap.h"
#include "Components/Image.h"

#include "Character/UK_PlayerController.h"

void UUK_MainMap::ApplyMapZoom(float InZoomLevel)
{
	CurrentZoom = FMath::Clamp(InZoomLevel, MinZoom, MaxZoom);

	if (MapImage)
	{
		MapImage->SetRenderScale(FVector2D(CurrentZoom, CurrentZoom));
	}
}

void UUK_MainMap::ResetMapZoom()
{
	ApplyMapZoom(1.0f);
}

void UUK_MainMap::OnMapViewOpened()
{
	ResetMapZoom();
}

void UUK_MainMap::NativeConstruct()
{
	Super::NativeConstruct();
	ResetMapZoom();

	if (MapImage)
	{
		MapImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUK_MainMap::NativeDestruct()
{
	Super::NativeDestruct();
}

FReply UUK_MainMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const float WheelDelta = InMouseEvent.GetWheelDelta();
	const float Direction = (WheelDelta > 0.f ) ? 1.f : -1.f;
	ApplyMapZoom(CurrentZoom + Direction * ZoomStep);
	return FReply::Handled();
}
