#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UK_InGameSave.generated.h"

USTRUCT(BlueprintType)
struct FQuestSaveData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> QuestProgress;
};

USTRUCT(BlueprintType)
struct FInventorySaveData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ItemIds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ItemCounts;
};

USTRUCT()
struct FCharacterStatSaveData
{
	GENERATED_BODY()
	UPROPERTY() float Health;
	UPROPERTY() float MaxHealth;
	UPROPERTY() float AttackPower;
	UPROPERTY() float CurrentPower;
	UPROPERTY() float CurrentMp;
	UPROPERTY() float MaxMp;
	UPROPERTY() float MaxStamina;
	UPROPERTY() float CurrentStamina;
	UPROPERTY() float Damage;
	UPROPERTY() int32 MaxLevel;
	UPROPERTY() int32 Level;
	UPROPERTY() int32 MaxEXP;
	UPROPERTY() float Exp;
	UPROPERTY() int32 Defence;
	UPROPERTY() int32 CriticalChance;
	UPROPERTY() int32 CriticalDamage;
};
UCLASS()
class UK_API UUK_InGameSave : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Player")
	FVector PlayerLocation;
	
	UPROPERTY(VisibleAnywhere, Category = "Player")
	FRotator PlayerRotation;
	
	UPROPERTY()
	FCharacterStatSaveData PlayerStats;
	
	UPROPERTY(VisibleAnywhere, Category = "Inv")
	FInventorySaveData InventoryDate;
	
	UPROPERTY(VisibleAnywhere, Category = "Quest")
	FQuestSaveData QuestDate;
	
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	FString SaveSlotName;
	
	UUK_InGameSave()
	{
		SaveSlotName = TEXT("UK_InGameSave");
	}
};
