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

	// 세이브 데이터 구조 버전(고정 상수)
	static constexpr int32 CURRENT_SAVE_VERSION = 0; 
	//개발단계이므로, 0으로 지정함. 안정성을 위해 flaot를 안쓰고 int를 사용했음

	// 이 슬롯이 저장될 당시의 버전 ("몇 버전의 구조로 저장됐는지")
	/* 단순 퀘스트 추가가 아닌, 퀘스트 진행 / 명명 / 작동방식 등이 변경될경우, 
	  오류방지를 위한 버전임*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveVersion = CURRENT_SAVE_VERSION;

	// SaveGame 전체의 마지막 저장 시각(디버그/운영 편의를 위한것,)
	// 주의. 퀘스트 하나가 언제 갱신됐는를 나타내는게 아님
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastSavedAt;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SavedMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUKPlayerSaveData HostPlayer;

	// 퀘스트 진행도 저장 (QuestID -> Progress)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FQuestProgress> QuestProgressMap;
};