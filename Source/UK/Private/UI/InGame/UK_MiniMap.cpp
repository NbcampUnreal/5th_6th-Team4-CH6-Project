#include "UI/InGame/UK_MiniMap.h"

#include "UI/InGame/UK_MapManager.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "UI/InGame/UK_MiniMapNorth.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

#include "Components/CanvasPanel.h"        
#include "Components/CanvasPanelSlot.h"    

void UUK_MiniMap::NativeConstruct()
{
	Super::NativeConstruct();

	AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer());

	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	//MID
	if (MiniMapImage)
	{
		MiniMapMID = MiniMapImage->GetDynamicMaterial();
	}

	if (!MiniMapMID) return;

	//MapManager 찾기
	MapManager = Cast<AUK_MapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUK_MapManager::StaticClass()));

	if (MapManager)
	{
		CachedMapData = MapManager->MapData;

		// 맵 텍스처 세팅
		if ( CachedMapData.MapTexture )
		{
			MiniMapMID->SetTextureParameterValue(MapTextureParam,Cast<UTexture>(CachedMapData.MapTexture.Get()));
		}
	}

	//타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle,
		this,
		&UUK_MiniMap::UpdateMiniMap,
		UpdateInterval,
		true
	);

	UpdateMiniMap();

	if (Compass_N) Compass_N->SetDirection(TEXT("N"));
	if (Compass_E) Compass_E->SetDirection(TEXT("E"));
	if (Compass_S) Compass_S->SetDirection(TEXT("S"));
	if (Compass_W) Compass_W->SetDirection(TEXT("W"));

}

void UUK_MiniMap::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::NativeDestruct();
}

void UUK_MiniMap::UpdateMiniMap()
{
	if (!MiniMapMID)
	{
		return;
	}

	AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	AUK_CharacterBase* Character = Cast<AUK_CharacterBase>(PC->GetPawn());
	if (!Character)
	{
		return;
	}

	//---------------- PlayerLocation ----------------
	const FVector Loc = Character->GetActorLocation();
	const FVector2D PlayerXY(Loc.X, Loc.Y);
	const FVector2D Delta = PlayerXY - CachedMapData.MapCenter;
	const float Size = FMath::Max(CachedMapData.MapSize, 1.f);
	const FVector2D Normalized(Delta.Y / Size, -Delta.X / Size);
	MiniMapMID->SetVectorParameterValue(PlayerLocationParam, FLinearColor(Normalized.X, Normalized.Y, 0.f, 0.f));

	//---------------- FacingDirectionYaw ----------------
	float Rot01 = 0.f;
	float YawDeg = PC->GetControlRotation().Yaw;

	YawDeg += YawOffsetDeg;

	const float Yaw360 = FMath::Fmod(YawDeg + 360.f, 360.f);
	Rot01 = Yaw360 / 360.f;

	if (CachedMapData.AlwaysFaceNroth)
	{
		Rot01 = 0.f;
	}

	MiniMapMID->SetScalarParameterValue(FacingDirectionYawParam, Rot01);

	//---------------- CompassRing Reverse Rotation ----------------
	if (CompassRingPanel)
	{
		const float RingYaw = -Yaw360;

		FWidgetTransform XForm = CompassRingPanel->GetRenderTransform();
		XForm.Angle = RingYaw;
		CompassRingPanel->SetRenderTransform(XForm);
		CompassRingPanel->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}

	if (MiniMapFrame)
	{
		const float RingYaw = -Yaw360;

		FWidgetTransform XForm = MiniMapFrame->GetRenderTransform();
		XForm.Angle = RingYaw;
		MiniMapFrame->SetRenderTransform(XForm);
		MiniMapFrame->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}
}
