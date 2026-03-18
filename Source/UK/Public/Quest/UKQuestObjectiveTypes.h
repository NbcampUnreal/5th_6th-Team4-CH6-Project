#pragma once

#include "CoreMinimal.h"
#include "UKQuestObjectiveTypes.generated.h"

// 이벤트 카테고리(명명규칙과 1:1 대응)
UENUM(BlueprintType)
enum class EUKQuestEventCategory : uint8
{
	EnteredZone,
	TalkedTo,
	GotItem,
	Killed,
	Custom,
	Delivered,

	Accepted,
	Completed,
	Failed,
};

// 목표 타입(퀘스트 정의에서 사용)
UENUM(BlueprintType)
enum class EUKQuestObjectiveType : uint8
{
	EnteredZone,
	TalkedTo,
	GotItem,
	Killed,
	Delivered,
	Custom,
};

// 목표 1개 정의
USTRUCT(BlueprintType)
struct FUKQuestObjectiveDef
{
	GENERATED_BODY()

	// UI/디버깅용 이름(추천: 규칙에 맞춰 Obj_<QuestID>_<Name> 형태를 넣어도 됨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ObjectiveId;

	// 목표 종류
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EUKQuestObjectiveType Type = EUKQuestObjectiveType::Custom;

	// 목표 타겟(예: NPC_Start_OldMan / Mob_Common_Wolf / Item_Steampunk_Permit / Start / Trig_...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TargetId;

	// 필요한 수치(대부분 KillCount/CollectCount 같은 카운터 목표)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredCount = 1;

	// 이 목표가 사용하는 카운터 이름(완전키는 QuestID와 합쳐서 CounterKey로 생성)
	// 예: KillWolves / CollectGear / DeliverItems
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CounterName;

	// 목표 완료 시 세팅할 FlagCategory(완전키는 QuestID와 합쳐서 FlagKey로 생성)
	// 예: ObjDone_KillWolves  (표준은 팀 규칙에 맞게 통일)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CompleteFlagCategory;
};
