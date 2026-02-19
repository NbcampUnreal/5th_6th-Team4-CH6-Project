#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest/UKQuestPresetTypes.h"       // EUKQuestTag
#include "Quest/UKQuestObjectiveTypes.h"    // Objectives
#include "UKQuestDefinitionAsset.generated.h"

UCLASS(BlueprintType)
class UK_API UUKQuestDefinitionAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// [A] Identity (명명규칙 기반)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|ID")
	FName QuestId; // Q_<Zone>_<M|S|T>_<Tag>_<NNN>

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|ID")
	FName ZoneName; // Start / Steampunk / Common

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|ID")
	EUKQuestTag Tag = EUKQuestTag::DIA;

	// [B] UI Meta (선택)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|UI")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|UI")
	FText Description;

	// [C] Objectives (핵심)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	TArray<FUKQuestObjectiveDef> Objectives;

	// [D] Conditions (참조만: Cond_...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Conditions")
	FName StartConditionId;    // Cond_Common_... 또는 Cond_<QuestID>_...

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Conditions")
	FName CompleteConditionId; // (선택)

	// [E] Rewards (참조만: R_...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards")
	FName RewardId; // R_<QuestID>_<Name> 또는 R_Common_<Name>


	// [F] Dialogue (JSON 참조만)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Dialogue")
	FName DialoguePackId; // 예: D.Start.M.001-004.json (파일명 or pack id)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Dialogue")
	FName DialogueId;     // 예: D_Q_Start_S_DLV_001_Offer
};