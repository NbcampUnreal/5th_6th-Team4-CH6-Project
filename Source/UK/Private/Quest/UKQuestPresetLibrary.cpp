#include "Quest/UKQuestPresetLibrary.h"

bool UUKQuestPresetLibrary::ApplyPresetToProgress(
	const UUKQuestPresetAsset* PresetAsset,
	EUKQuestTag Tag,
	FQuestProgress& InOutProgress
)
{
	if ( !PresetAsset ) return false;

	const FUKQuestPresetData* Preset = PresetAsset->Presets.Find(Tag);
	if ( !Preset ) return false;

	// Step 기본값
	InOutProgress.Step = Preset->DefaultStartStep;

	// 기본 Flag/Counters 초기화(“이름만” 넣고, 실제 키 규칙은 별도 시스템에서 확장 가능)
	// 여기서는 Progress 내부의 컨테이너에 “기본 요소”를 만들어주는 수준만.
	for ( const FName& CName : Preset->DefaultCounterNames )
	{
		InOutProgress.Counters.FindOrAdd(CName) = 0;
	}

	for ( const FName& FCategory : Preset->DefaultFlagCategories )
	{
		InOutProgress.Flags.Add(FCategory);
	}

	// Completed는 기본 false 유지
	InOutProgress.bCompleted = false;

	return true;
}