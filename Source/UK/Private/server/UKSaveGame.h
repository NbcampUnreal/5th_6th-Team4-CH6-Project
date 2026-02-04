#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UKSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FUKPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
};

UCLASS()
class UK_API UUKSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SavedMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUKPlayerSaveData HostPlayer;
};