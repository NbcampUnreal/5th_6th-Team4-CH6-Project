#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UKQuestRewardTypes.generated.h"

// 아이템 지급 1개 단위(아이템ID + 수량)
USTRUCT(BlueprintType)
struct FUKRewardItemGrant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemId;          // Item_<Zone>_<Name>
							//ex) Item_Steampunk_Permit 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Amount = 1;
};

// Reward DataTable Row
USTRUCT(BlueprintType)
struct FUKRewardRow : public FTableRowBase
{
	GENERATED_BODY()

	// 기본 보상(있으면 적용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 XP = 0;

	// 아이템 보상(여러 개)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FUKRewardItemGrant> Items;

	// 플래그 세트(전역/퀘스트 둘 다 가능)
	// 예: F.Common.Chapter2Unlocked, F.Q_Start_T_HNT_002.CompletedBonus 등
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> SetFlags;

	// 카운터 증가(전역/퀘스트 둘 다 가능)
	// 예: C.Common.TotalWolfKills -> +1
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> AddCounters;
};
