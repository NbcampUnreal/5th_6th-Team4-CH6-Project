#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_MiniMap.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class AUK_MapManager;

UCLASS()
class UK_API UUK_MiniMap : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UImage* MiniMapImage;

	UPROPERTY()
	UMaterialInstanceDynamic* MinimapDynamicMaterial;

	UPROPERTY()
	AUK_MapManager* CachedMapManager;
};
