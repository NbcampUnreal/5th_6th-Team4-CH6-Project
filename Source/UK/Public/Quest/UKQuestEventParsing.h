#pragma once

#include "CoreMinimal.h"
#include "Quest/UKQuestObjectiveTypes.h"

// EventId 규칙: QuestEvent.<Category>.<Detail>
struct FUKQuestParsedEvent
{
	bool bValid = false;
	EUKQuestEventCategory Category = EUKQuestEventCategory::Custom;
	FName Detail; // ZoneName / NPCId / ItemId / MobId / TriggerId / QuestID 등
};

static inline bool UK_ParseQuestEventId(const FName EventId, FUKQuestParsedEvent& Out)
{
	Out = {};

	const FString S = EventId.ToString();
	TArray<FString> Parts;
	S.ParseIntoArray(Parts, TEXT("."), true);

	// QuestEvent.<Category>.<Detail> => 3 토큰
	if ( Parts.Num() < 3 ) return false;
	if ( Parts[ 0 ] != "QuestEvent" ) return false;

	const FString& Cat = Parts[ 1 ]; // Cat = <Category>
	const FString& Det = Parts[ 2 ]; // Det = <Detail>

	auto SetCat = [ & ] (EUKQuestEventCategory C)
		{
			Out.bValid = true;
			Out.Category = C;
			Out.Detail = FName(*Det);
		};

	// 카테고리 확인
	if ( Cat == "EnteredZone" ) { SetCat(EUKQuestEventCategory::EnteredZone); return true; }
	if ( Cat == "TalkedTo" ) { SetCat(EUKQuestEventCategory::TalkedTo); return true; }
	if ( Cat == "GotItem" ) { SetCat(EUKQuestEventCategory::GotItem); return true; }
	if ( Cat == "Killed" ) { SetCat(EUKQuestEventCategory::Killed); return true; }
	if ( Cat == "Custom" ) { SetCat(EUKQuestEventCategory::Custom); return true; }

	if ( Cat == "Accepted" ) { SetCat(EUKQuestEventCategory::Accepted); return true; }
	if ( Cat == "Completed" ) { SetCat(EUKQuestEventCategory::Completed); return true; }
	if ( Cat == "Failed" ) { SetCat(EUKQuestEventCategory::Failed); return true; }

	return false;
}