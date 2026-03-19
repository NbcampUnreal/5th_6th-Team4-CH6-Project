#include "UI/InGame/UK_MainMap.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/CanvasPanel.h"    
#include "Components/CanvasPanelSlot.h"  
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"

#include "Character/UK_PlayerController.h"
#include "Character/UK_CharacterBase.h" 
#include "UI/InGame/UK_MapManager.h"    
#include "Kismet/GameplayStatics.h"      
#include "TimerManager.h"        

#include "UI/InGame/UK_WarpIcon.h"
#include "Level/Warp/UK_WarpSubsystem.h"

void UUK_MainMap::NativeConstruct()
{
	Super::NativeConstruct();

	// 서브시스템 가져오기
	UUK_WarpSubsystem* WarpSubsystem = GetWorld()->GetSubsystem<UUK_WarpSubsystem>();

	if ( WarpSubsystem && WarpDataTableAsset )
	{
		// 서브시스템의 WarpDataTable에 내가 들고 있는 에셋을 넣어줌
		WarpSubsystem->WarpDataTable = WarpDataTableAsset;
		UE_LOG(LogTemp, Log, TEXT("WarpDataTable has been successfully linked to Subsystem!"));

		// 로딩창 클래스 
		WarpSubsystem->LoadingWidgetClass = LoadingWidgetClass;
	}

	if (HorizontalScrollBox)
	{
		HorizontalScrollBox->SetOrientation(EOrientation::Orient_Horizontal);
		HorizontalScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		HorizontalScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Never);
		HorizontalScrollBox->SetAllowRightClickDragScrolling(false);
	}

	if (VerticalScrollBox)
	{
		VerticalScrollBox->SetOrientation(EOrientation::Orient_Vertical);
		VerticalScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		VerticalScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Never);
		VerticalScrollBox->SetAllowRightClickDragScrolling(false);
	}

	CacheMapBaseSize();
	ResetMapZoom();

	if (MapImage)
	{
		MapImage->SetVisibility(ESlateVisibility::Visible);
	}

	MapManager = Cast<AUK_MapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUK_MapManager::StaticClass()));

	if (MapManager)
	{
		CachedMapData = MapManager->MapData;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			PlayerUpdateTimerHandle,
			this,
			&UUK_MainMap::UpdatePlayerLocation,
			UpdateInterval,
			true
		);
	}

	UpdatePlayerLocation();

	InitializeWarpIcons(); // 아이콘 생성
	UpdateWarpIconLocations(); // 첫 위치 잡기

}
void UUK_MainMap::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PlayerUpdateTimerHandle);
	}

	Super::NativeDestruct();
}

//----플레이어 위치 업데이트 로직---
void UUK_MainMap::UpdatePlayerLocation()
{
	if (!PlayerIcon || !MapImage || !MapContainer) return;

	AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer());
	if (!PC) return;

	AUK_CharacterBase* Character = Cast<AUK_CharacterBase>(PC->GetPawn());
	if (!Character)return;

	//-------- 월드 좌표 -> 맵 좌표 변환 --------
	const FVector Loc = Character->GetActorLocation();
	const FVector2D PlayerXY(Loc.X, Loc.Y);

	const FVector2D Delta = PlayerXY - CachedMapData.MapCenter;
	const float WorldSize = FMath::Max(static_cast< float >( CachedMapData.MapSize ), 1.f);

	float U = ( Delta.Y / WorldSize ) + 0.5f;
	float V = ( Delta.X / WorldSize ) + 0.5f;
	V = 1.0f - V;

	//맵 밖으로 나가면 아이콘이 아예 밖으로 튀는거 방지
	U = FMath::Clamp(U, 0.f, 1.f);
	V = FMath::Clamp(V, 0.f, 1.f);

	//MapImage 기준 좌표로 찍고 -> MapContainer 좌표로 변환
	const FGeometry& MapGeom = MapImage->GetCachedGeometry();
	const FVector2D MapSize = MapGeom.GetLocalSize();
	if ( MapSize.X <= 1.f || MapSize.Y <= 1.f ) return;

	//MapImage 로컬 픽셀
	const FVector2D MapLocalPixel(U * MapSize.X, V * MapSize.Y);

	//MapImage 로컬 -> Absolute
	const FVector2D AbsPos = MapGeom.LocalToAbsolute(MapLocalPixel);

	//Absolute -> MapContainer 로컬
	const FGeometry& ContGeom = MapContainer->GetCachedGeometry();
	const FVector2D ContSize = ContGeom.GetLocalSize();
	if ( ContSize.X <= 1.f || ContSize.Y <= 1.f ) return;

	FVector2D ContLocalPos = ContGeom.AbsoluteToLocal(AbsPos);

	//-------- 아이콘 배치 --------
	if (UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(PlayerIcon->Slot))
	{
		IconSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		IconSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		IconSlot->SetPosition(ContLocalPos);
	}

	//-------- 아이콘 회전 --------
	const float Yaw = PC->GetControlRotation().Yaw;
	PlayerIcon->SetRenderTransformAngle(Yaw + 90.0f);

	UpdateWarpIconLocations();
}
void UUK_MainMap::CacheMapBaseSize()
{
	if (!MapImage) return;

	FVector2D Size = MapImage->GetCachedGeometry().GetLocalSize();

	if (Size.X > 0.f && Size.Y > 0.f)
	{
		MapBaseSize = Size;
		return;
	}

	FVector2D BrushSize = MapImage->GetBrush().GetImageSize();
	if (BrushSize.X > 0.f && BrushSize.Y > 0.f)
	{
		MapBaseSize = BrushSize;
		return;
	}

	UObject* Resource = MapImage->GetBrush().GetResourceObject();
	if (Resource)
	{
		UTexture2D* Texture = Cast<UTexture2D>(Resource);
		if (Texture)
		{
			MapBaseSize.X = Texture->GetSizeX();
			MapBaseSize.Y = Texture->GetSizeY();
			return;
		}
	}

	MapBaseSize = FVector2D(1920.f, 1080.f);
}
void UUK_MainMap::UpdateMapTransform()
{
	if ( !MapImage ) return;

	float NewWidth = MapBaseSize.X * CurrentZoom;
	float NewHeight = MapBaseSize.Y * CurrentZoom;

	//SizeBox로 ScrollBox에 크기 전달
	if ( MapSizeBox )
	{
		MapSizeBox->SetWidthOverride(NewWidth);
		MapSizeBox->SetHeightOverride(NewHeight);
	}
}
//----줌 관련 로직들---
void UUK_MainMap::ApplyMapZoom(float InZoomLevel, FVector2D LocalMousePos)
{
	float OldZoom = CurrentZoom;
	float NewZoom = FMath::Clamp(InZoomLevel, MinZoom, MaxZoom); 

	if (FMath::IsNearlyEqual(OldZoom, NewZoom)) return;

	//마우스 위치 기준으로 줌이 되도록 스크롤 위치 조정
	float OldScrollX = HorizontalScrollBox ? HorizontalScrollBox->GetScrollOffset() : 0.f;//
	float OldScrollY = VerticalScrollBox ? VerticalScrollBox->GetScrollOffset() : 0.f;

	float ContentX = (OldScrollX + LocalMousePos.X) / OldZoom;
	float ContentY = (OldScrollY + LocalMousePos.Y) / OldZoom;

	CurrentZoom = NewZoom;
	UpdateMapTransform();
	
	float NewScrollX = ContentX * NewZoom - LocalMousePos.X;
	float NewScrollY = ContentY * NewZoom - LocalMousePos.Y;

	if (HorizontalScrollBox)
	{
		HorizontalScrollBox->SetScrollOffset(NewScrollX);
	}

	if (VerticalScrollBox)
	{
		VerticalScrollBox->SetScrollOffset(NewScrollY);
	}

	UpdateWarpIconLocations();

	UpdatePlayerLocation();
}
void UUK_MainMap::ResetMapZoom()
{
	CurrentZoom = 1.0f;

	if (HorizontalScrollBox)
	{
		HorizontalScrollBox->SetScrollOffset(0.f);
	}

	if (VerticalScrollBox)
	{
		VerticalScrollBox->SetScrollOffset(0.f);
	}

	UpdateMapTransform();
	UpdatePlayerLocation();
}
void UUK_MainMap::OnMapViewOpened()
{
	ResetMapZoom();
}
//--- 마우스 관련 로직들 ---
FReply UUK_MainMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsMouseInsideMap(InGeometry, InMouseEvent))
	{
		return FReply::Unhandled();
	}

	FVector2D LocalPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	const float WheelDelta = InMouseEvent.GetWheelDelta();
	const float Direction = ( WheelDelta > 0.f ) ? 1.f : -1.f;
	ApplyMapZoom(CurrentZoom + Direction * ZoomStep, LocalPos);
	return FReply::Handled();
}
FReply UUK_MainMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!IsMouseInsideMap(InGeometry, InMouseEvent))
		{
			return FReply::Unhandled();
		}

		if (FMath::IsNearlyEqual(CurrentZoom, 1.0f))
		{
			return FReply::Unhandled();
		}

		bIsDragging = true;
		LastMousePosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(GetCachedWidget().ToSharedRef());
	}
	return FReply::Unhandled();
}
FReply UUK_MainMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}
FReply UUK_MainMap::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();
		FVector2D Delta = CurrentMousePos - LastMousePosition;
		LastMousePosition = CurrentMousePos;

		if (HorizontalScrollBox)
		{
			float CurrentX = HorizontalScrollBox->GetScrollOffset();
			HorizontalScrollBox->SetScrollOffset(CurrentX - Delta.X);
		}

		if (VerticalScrollBox)
		{
			float CurrentY = VerticalScrollBox->GetScrollOffset();
			VerticalScrollBox->SetScrollOffset(CurrentY - Delta.Y);
		}

		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}
bool UUK_MainMap::IsMouseInsideMap(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const
{
	FVector2D LocalPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D WidgetSize = InGeometry.GetLocalSize();

	return LocalPos.X >= 0.f && LocalPos.X <= WidgetSize.X && LocalPos.Y >= 0.f && LocalPos.Y <= WidgetSize.Y;
}

// 월드 좌표를 맵 UI 좌표로 변환하는 공통 함수
FVector2D UUK_MainMap::GetMapPositionFromWorld(const FVector& WorldPos)
{
	if ( !MapImage || !MapContainer ) return FVector2D::ZeroVector;

	const FVector2D WorldXY(WorldPos.X, WorldPos.Y);
	const FVector2D Delta = WorldXY - CachedMapData.MapCenter;
	const float WorldSize = FMath::Max(static_cast< float >( CachedMapData.MapSize ), 1.f);

	float U = ( Delta.Y / WorldSize ) + 0.5f;
	float V = ( Delta.X / WorldSize ) + 0.5f;
	V = 1.0f - V;

	U = FMath::Clamp(U, 0.f, 1.f);
	V = FMath::Clamp(V, 0.f, 1.f);

	const FGeometry& MapGeom = MapImage->GetCachedGeometry();
	const FVector2D MapSize = MapGeom.GetLocalSize();
	if ( MapSize.X <= 1.f || MapSize.Y <= 1.f ) return FVector2D::ZeroVector;

	const FVector2D MapLocalPixel(U * MapSize.X, V * MapSize.Y);
	const FVector2D AbsPos = MapGeom.LocalToAbsolute(MapLocalPixel);
	const FGeometry& ContGeom = MapContainer->GetCachedGeometry();

	return ContGeom.AbsoluteToLocal(AbsPos);
}

// 데이터테이블을 읽어 아이콘 위젯들을 최초 생성
void UUK_MainMap::InitializeWarpIcons()
{
	UUK_WarpSubsystem* WarpSubsystem = GetWorld()->GetSubsystem<UUK_WarpSubsystem>();

	//  Subsystem이나 DataTable이 없으면 그냥 리턴해서 크래시를 막음.
	if ( !WarpSubsystem || !WarpSubsystem->WarpDataTable || !WarpIconClass )
	{
		UE_LOG(LogTemp, Error, TEXT("WarpDataTable or WarpIconClass is NULL!"));
		return;
	}
	
    // 기존 생성된 위젯이 있다면 제거
	for ( auto Icon : WarpIconWidgets ) { if ( Icon ) Icon->RemoveFromParent(); }
	WarpIconWidgets.Empty();

	TArray<FWarpPointRow*> AllRows;
	WarpSubsystem->WarpDataTable->GetAllRows<FWarpPointRow>(TEXT("WarpInit"), AllRows);
	TArray<FName> ActivatedIDs = WarpSubsystem->GetActivatedPointIDs();

	for ( FWarpPointRow* Row : AllRows )
	{
		UUK_WarpIcon* NewIcon = CreateWidget<UUK_WarpIcon>(this, WarpIconClass);
		if ( NewIcon )
		{
			bool bIsActive = ActivatedIDs.Contains(Row->WarpPointID);
			NewIcon->InitWarpIcon(Row->WarpPointID, bIsActive);

			MapContainer->AddChildToCanvas(NewIcon);
			WarpIconWidgets.Add(NewIcon);
		}
	}
}

// 줌이나 드래그 시 워프 아이콘들의 위치를 재계산
void UUK_MainMap::UpdateWarpIconLocations()
{
	UUK_WarpSubsystem* WarpSubsystem = GetWorld()->GetSubsystem<UUK_WarpSubsystem>();
	if ( !WarpSubsystem ) return;

	for ( UUK_WarpIcon* Icon : WarpIconWidgets )
	{
		if ( !Icon ) continue;

		FWarpPointRow RowData = WarpSubsystem->GetWarpRowByID(Icon->WarpPointID);
		// 데이터테이블에 추가한 WorldLocation 좌표 사용
		FVector2D MapPos = GetMapPositionFromWorld(RowData.WorldLocation);

		if ( UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(Icon->Slot) )
		{
			IconSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
			IconSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			IconSlot->SetPosition(MapPos);
		}
	}
}