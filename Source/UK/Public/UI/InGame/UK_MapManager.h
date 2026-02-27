#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_MapManager.generated.h"

class USceneCaptureComponent2D;
class AUK_CharacterBase;

USTRUCT(BlueprintType)
struct FUK_MapData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	float MapSize;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	FVector2D MapCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	TObjectPtr<UTextureRenderTarget2D> MapTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	float CaptureHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	bool AlwaysFaceNroth;

	FUK_MapData()
		: MapSize(400000.0f)
		, MapCenter(FVector2D::ZeroVector)
		, MapTexture(nullptr)
		, CaptureHeight(10000.0f)
		, AlwaysFaceNroth(true)
	{}
};

UCLASS()
class UK_API AUK_MapManager : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AUK_MapManager();

	virtual void BeginPlay() override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	FUK_MapData MapData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MapComponents")
	TObjectPtr<USceneCaptureComponent2D> MapCaptureComponent;

	UFUNCTION(BlueprintCallable, Category = "MapCapture")
	void CaptureMap();

	void UpdateCaptureTransform();

	FTimerHandle CaptureTimer;


//에디터에서 MapData가 변경될 때마다 맵 캡처를 갱신하도록 설정
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
