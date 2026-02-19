#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest/UKQuestPresetTypes.h"
#include "UKQuestPresetAsset.generated.h"

// 태그별 프리셋 데이터 묶음
UCLASS(BlueprintType)
class UK_API UUKQuestPresetAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 태그별 기본 프리셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EUKQuestTag, FUKQuestPresetData> Presets;
};