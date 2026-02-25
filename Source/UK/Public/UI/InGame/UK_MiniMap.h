#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/InGame/UK_MapManager.h"
#include "UK_MiniMap.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class AUK_MapManager;
class UUK_MiniMapNorth;
class UCanvasPanel;


UCLASS()
class UK_API UUK_MiniMap : public UUserWidget
{
	GENERATED_BODY()

protected:
	//---------- UUserWidget Interface ----------//
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//---------- MiniMap Update ----------//
	UFUNCTION()
	void UpdateMiniMap();

protected:
	//---------- Widget References ----------//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MiniMapImage;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MiniMapMID;
	UPROPERTY()
	TObjectPtr<AUK_MapManager> MapManager;

	//---------- Cached Map Data ----------//
	UPROPERTY()
	struct FUK_MapData CachedMapData;
	FTimerHandle UpdateTimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap|Rotation")
	float YawOffsetDeg = 90.f;
	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float UpdateInterval = 0.05f; // 20Hz

	//---------- Material Parameter Names ----------//
	UPROPERTY(EditAnywhere, Category = "MiniMap|Material")
	FName MapTextureParam = TEXT("MapTexture");

	UPROPERTY(EditAnywhere, Category = "MiniMap|Material")
	FName PlayerLocationParam = TEXT("PlayerLocation");

	UPROPERTY(EditAnywhere, Category = "MiniMap|Material")
	FName FacingDirectionYawParam = TEXT("FacingDirectionYaw");

	//---------- Compass Widgets ----------//
	UPROPERTY(meta = ( BindWidgetOptional ))
	TObjectPtr<UUK_MiniMapNorth> Compass_N;

	UPROPERTY(meta = ( BindWidgetOptional ))
	TObjectPtr<UUK_MiniMapNorth> Compass_E;

	UPROPERTY(meta = ( BindWidgetOptional ))
	TObjectPtr<UUK_MiniMapNorth> Compass_S;

	UPROPERTY(meta = ( BindWidgetOptional ))
	TObjectPtr<UUK_MiniMapNorth> Compass_W;

	UPROPERTY(meta = ( BindWidgetOptional ))
	TObjectPtr<UWidget> CompassRingPanel;

	UPROPERTY(meta = ( BindWidgetOptional ))
	TObjectPtr<UWidget> MiniMapFrame;

};