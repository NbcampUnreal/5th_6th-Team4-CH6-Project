#pragma once
#include "CoreMinimal.h"
#include "UK_MonsterTypes.generated.h"

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	None             = 0,
	Grux             = 1,
	EliteGolem       = 2,
	EliteWolf        = 3,
	EliteInsectBeast = 4,
	Golem            = 5,
	Wolf             = 6,
	Fox              = 7,
	Reindeer         = 8,
	InsectBeast      = 9,
	Centipede		 =10,
};

UENUM(BlueprintType)
enum class EMonsterRank : uint8
{
	Normal = 0,
	Elite  = 1,
	Boss   = 2,
};

UENUM(BlueprintType)
enum class EHitReactionType : uint8
{
	None           = 0,  // 반응 없음
	Flinch         = 1,  // 움찔
	KnockbackWalk  = 2,  // 밀림
	KnockbackFall  = 3,  // 넘어짐
};

UENUM(BlueprintType)
enum class EMonsterAttackType : uint8
{
	Normal  = 0,
	Special = 1,
};