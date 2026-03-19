#include "Quest/UKQuestPresetLibrary.h"

namespace
{
	static FName MakePresetCounterKey(FName QuestId, FName CounterName)
	{
		// C.<QuestID>.<Name>
		return FName(*FString::Printf(TEXT("C.%s.%s"),
			*QuestId.ToString(),
			*CounterName.ToString()));
	}

	static FName MakePresetFlagKey(FName QuestId, FName Category)
	{
		// F.<QuestID>.<Category>
		return FName(*FString::Printf(TEXT("F.%s.%s"),
			*QuestId.ToString(),
			*Category.ToString()));
	}

	static void AddCommonFlags(FQuestProgress& InOutProgress, FName QuestId)
	{
		// 시작 시점에는 Accepted만 넣는다.
		// Completed / Failed 는 실제 상태가 바뀔 때만 넣어야 한다.
		InOutProgress.Flags.Add(MakePresetFlagKey(QuestId, FName("Accepted")));
	}
}

bool UUKQuestPresetLibrary::ApplyPresetToProgress(
	const UUKQuestPresetAsset* PresetAsset,
	FName QuestId,
	EUKQuestTag Tag,
	FQuestProgress& InOutProgress
)
{
	if ( !PresetAsset ) return false;
	if ( QuestId.IsNone() ) return false;

	const FUKQuestPresetData* Preset = PresetAsset->Presets.Find(Tag);
	if ( !Preset ) return false;

	// 1) 기본 시작 Step
	InOutProgress.Step = Preset->DefaultStartStep;

	// 2) 공통 상태 플래그 자동 생성
	if ( Preset->bAutoInitCommonFlags )
	{
		AddCommonFlags(InOutProgress, QuestId);
	}

	// 3) 태그별 기본 Counter 자동 생성
	for ( const FName& CounterName : Preset->DefaultCounterNames )
	{
		if ( CounterName.IsNone() )
		{
			continue;
		}

		const FName CounterKey = MakePresetCounterKey(QuestId, CounterName);
		InOutProgress.Counters.FindOrAdd(CounterKey) = 0;
	}

	// 4) 태그별 기본 Flag 자동 생성
	for ( const FName& FlagCategory : Preset->DefaultFlagCategories )
	{
		if ( FlagCategory.IsNone() )
		{
			continue;
		}

		const FName FlagKey = MakePresetFlagKey(QuestId, FlagCategory);
		InOutProgress.Flags.Add(FlagKey);
	}

	// 5) 시작 시점 완료 상태는 false
	InOutProgress.bCompleted = false;

	return true;
}