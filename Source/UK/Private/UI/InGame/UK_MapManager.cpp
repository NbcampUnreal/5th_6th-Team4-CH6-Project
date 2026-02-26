#include "UI/InGame/UK_MapManager.h"
#include "Character/UK_PlayerController.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TimerManager.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

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

	GetWorldTimerManager().SetTimer(CaptureTimer, this, &AUK_MapManager::CaptureMap, 0.2f, true);
}

void AUK_MapManager::CaptureMap()
{
	if (!MapCaptureComponent) return;

	RefreshHiddenActors();

	UpdateCaptureTransform();

	MapCaptureComponent->OrthoWidth = MapData.MapSize;

	if ( MapData.MapTexture )
	{
		MapCaptureComponent->TextureTarget = MapData.MapTexture.Get();
	}

	MapCaptureComponent->CaptureScene();
}

void AUK_MapManager::UpdateCaptureTransform()
{
	SetActorLocation(FVector(MapData.MapCenter.X, MapData.MapCenter.Y, MapData.CaptureHeight));
	SetActorRotation(FRotator(-90.f, 0.f, 0.f));
}

void AUK_MapManager::RefreshHiddenActors()
{
	if (!MapCaptureComponent) return;

	MapCaptureComponent->HiddenActors.Empty();

	// 월드의 모든 Pawn(플레이어 포함)을 숨김 처리
	TArray<AActor*> Pawns;
	UGameplayStatics::GetAllActorsOfClass (
		GetWorld(),
		APawn::StaticClass(),
		Pawns
	);

	for (AActor* Pawn : Pawns)
	{
		if (Pawn)
		{
			MapCaptureComponent->HiddenActors.Add(Pawn);
		}
	}
}

#if WITH_EDITOR
void AUK_MapManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	CaptureMap();
}
#endif