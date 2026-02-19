#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Quest/UKQuestPresetTypes.h"
#include "Quest/UKQuestTypes.h"              // FQuestProgress
#include "Quest/UKQuestPresetAsset.h"
#include "UKQuestPresetLibrary.generated.h"

UCLASS()
class UK_API UUKQuestPresetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 프리셋을 진행도에 “초기 적용”
	UFUNCTION(BlueprintCallable, Category = "Quest|Preset")
	static bool ApplyPresetToProgress(
		const UUKQuestPresetAsset* PresetAsset,
		EUKQuestTag Tag,
		FQuestProgress& InOutProgress
	);
};
