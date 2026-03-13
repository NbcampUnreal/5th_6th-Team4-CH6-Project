#pragma once
#include "Engine/DataTable.h"
#include "AIMonster/UK_MonsterTypes.h"
#include "UK_MonsterCombatRow.generated.h"

/** 행 키 = EMonsterType 이름 (ex. "Wolf", "EliteGolem") */
USTRUCT(BlueprintType)
struct FUK_MonsterCombatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
	EHitReactionType NormalAttackHit = EHitReactionType::Flinch;

	/** 엘리트/보스 전용, Normal 몬스터는 None */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
	EHitReactionType SpecialAttackHit = EHitReactionType::None;
};