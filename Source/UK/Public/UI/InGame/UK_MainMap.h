#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_MainMap.generated.h"

class UImage;

UCLASS()
class UK_API UUK_MainMap : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 줌 설정
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float MinZoom = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom",meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float MaxZoom = 4.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom",meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float ZoomStep = 0.1f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zoom")
	float CurrentZoom = 1.0f;

	//바인드 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MapImage;


	// 줌 적용 함수
	UFUNCTION(BlueprintCallable, Category = "Map")
	void ApplyMapZoom(float InZoomLevel);

	UFUNCTION(BlueprintCallable, Category = "Map")
	void ResetMapZoom();

	UFUNCTION(BlueprintCallable, Category = "Map")
	void OnMapViewOpened();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

};
