#include "UI/InGame/UK_MapManager.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TimerManager.h"


AUK_MapManager::AUK_MapManager()
{
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootSceneComponent);
	//맵 컴포넌트
	MapCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MapCaptureComponent"));
	MapCaptureComponent->SetupAttachment(RootSceneComponent);

	//맵 캡처 컴포넌트 설정
	MapCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	MapCaptureComponent->OrthoWidth = 400000.0f;//기본값
	MapCaptureComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	MapCaptureComponent->bCaptureEveryFrame = false;
	MapCaptureComponent->bCaptureOnMovement = false;
}

void AUK_MapManager::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(CaptureTimer, this, &AUK_MapManager::CaptureMap, 0.2f, false);
}

void AUK_MapManager::CaptureMap()
{
	if (!MapCaptureComponent) return;

	UpdateCaptureTransform();

	MapCaptureComponent->OrthoWidth = MapData.MapSize;
	MapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	//노출 고정
	MapCaptureComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
	MapCaptureComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	MapCaptureComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
	MapCaptureComponent->PostProcessSettings.AutoExposureBias = 0.f;

	if (UTextureRenderTarget2D* RTMap = Cast<UTextureRenderTarget2D>(MapData.MapTexture))
	{
		MapCaptureComponent->TextureTarget = RTMap;
	}
	MapCaptureComponent->CaptureScene();
}

void AUK_MapManager::UpdateCaptureTransform()
{
	SetActorLocation(FVector(MapData.MapCenter.X, MapData.MapCenter.Y, MapData.CaptureHeight));
	SetActorRotation(FRotator(-90.f, 0.f, 0.f));
}

#if WITH_EDITOR
void AUK_MapManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	CaptureMap();
}
#endif