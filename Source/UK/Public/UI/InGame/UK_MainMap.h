#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/InGame/UK_MapManager.h"
#include "UK_MainMap.generated.h"

class UImage;
class UScrollBox;
class AUK_MapManager;
class UCanvasPanel;
class USizeBox;
class UUK_WarpIcon;

UCLASS()
class UK_API UUK_MainMap : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//---------- Map Zoom Settings ----------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float MinZoom = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom",meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float MaxZoom = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom",meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float ZoomStep = 0.1f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zoom")
	float CurrentZoom = 1.0f;

	//---------- Map Widget References ----------//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MapImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> HorizontalScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> VerticalScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> MapSizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MapContainer;

	//---------- Player Icon Settings ----------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FVector2D PlayerIconSize = FVector2D(24.f, 24.f);


	//---------- Map Interaction Functions ----------//
	UFUNCTION(BlueprintCallable, Category = "Map")
	void ApplyMapZoom(float InZoomLevel, FVector2D LocalMousePos);

	UFUNCTION(BlueprintCallable, Category = "Map")
	void ResetMapZoom();

	UFUNCTION(BlueprintCallable, Category = "Map")
	void OnMapViewOpened();

	//---------- Warp ----------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Data")
	TObjectPtr<UDataTable> WarpDataTableAsset;

	UPROPERTY(EditAnywhere, Category = "Map|Data")
	TSubclassOf<class UUK_Out_Loading> LoadingWidgetClass;

protected:
	//---------- UUserWidget Interface ----------//
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//---------- Map Interaction ----------//
	FVector2D MapBaseSize = FVector2D::ZeroVector;
	bool bIsDragging = false;
	FVector2D LastMousePosition = FVector2D::ZeroVector;

	void CacheMapBaseSize();
	void UpdateMapTransform();
	bool IsMouseInsideMap(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const;

	//---------- Player Location ----------//
	UPROPERTY() 
	TObjectPtr<AUK_MapManager> MapManager;

	UPROPERTY()
	struct FUK_MapData CachedMapData;

	FTimerHandle PlayerUpdateTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Map")
	float UpdateInterval = 0.1f;

	UFUNCTION()
	void UpdatePlayerLocation();

	//---------- Warp ----------//

	// 블루프린트에서 WBP_WarpIcon을 할당하기 위한 변수
	UPROPERTY(EditAnywhere, Category = "Map|Warp")
	TSubclassOf<UUK_WarpIcon> WarpIconClass;

	// 생성된 워프 아이콘들을 관리할 배열
	UPROPERTY()
	TArray<TObjectPtr<UUK_WarpIcon>> WarpIconWidgets;

	// 월드 좌표 -> 맵 로컬 좌표 변환 (기존 로직 통합)
	FVector2D GetMapPositionFromWorld(const FVector& WorldPos);

	// 워프 아이콘 초기 생성 함수
	void InitializeWarpIcons();

	// 워프 아이콘들 위치 업데이트 (줌/드래그 대응)
	void UpdateWarpIconLocations();

};
