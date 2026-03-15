#pragma once
#include "Engine/DataTable.h"
#include "UK_MonsterLootRow.generated.h"

USTRUCT(BlueprintType)
struct FUK_MonsterLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct FUK_MonsterLootRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FUK_MonsterLootEntry> Entries;
};