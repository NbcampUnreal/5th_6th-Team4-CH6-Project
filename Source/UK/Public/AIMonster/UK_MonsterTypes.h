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
	SandGolem        = 5,
	BlueGolem        = 6,
	GreenGolem       = 7,
	Wolf             = 8,
	Fox              = 9,
	Reindeer         =10,
	InsectBeast      =11,
	Centipede		 =12,
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