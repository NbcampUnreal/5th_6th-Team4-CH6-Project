#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UKQuestCookAnchor.generated.h"

class UUKQuestDefinitionAsset;
class UUKQuestConditionAsset;
class UUKQuestPresetAsset;
class UDataTable;

UCLASS(BlueprintType)
class UK_API UUKQuestCookAnchor : public UDataAsset
{
	GENERATED_BODY()

public:
	// 퀘스트 정의들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TArray<TObjectPtr<UUKQuestDefinitionAsset>> QuestDefinitions;

	// 조건 정의들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TArray<TObjectPtr<UUKQuestConditionAsset>> ConditionDefinitions;

	// 프리셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TObjectPtr<UUKQuestPresetAsset> PresetAsset = nullptr;

	// 데이터 테이블들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TObjectPtr<UDataTable> NPCDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TObjectPtr<UDataTable> MonsterDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TObjectPtr<UDataTable> RewardDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Cook")
	TObjectPtr<UDataTable> ItemDataTable = nullptr;
};