#pragma once
#include "Engine/DataTable.h"
#include "AIMonster/UK_MonsterTypes.h"
#include "UK_MonsterMetaRow.generated.h"

USTRUCT(BlueprintType)
struct FUK_MonsterMetaRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	EMonsterType MonsterType = EMonsterType::None;

	/** 일반 / 엘리트 / 보스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	EMonsterRank Rank = EMonsterRank::Normal;

	/**
	 * 퀘스트 명명규칙: 
	 Mob_<Zone>_<Name>
	 Elite_<Zone>_<Name>
	 Boss_<Zone>_<Name>
	 * ex) Mob_Common_Wolf
	 * → QuestEvent.Killed.Mob_Common_Wolf 자동 생성에 사용
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta|Quest")
	FName MobEntityId = NAME_None;

	/** 명명규칙 <Zone> */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta|Quest")
	FName Zone = "Common";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta|Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	float CorpseLingerTime = 5.f;
};