#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest/UKQuestTypes.h"  // 퀘스트 관련
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

	// 퀘스트 진행도 저장 (QuestID -> Progress)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FQuestProgress> QuestProgressMap;

	//// ----- Setting - Sound -----
	//float BackGround_Sound;
	//float SFX_Sound;
};