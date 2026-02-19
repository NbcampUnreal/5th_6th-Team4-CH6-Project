#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UKQuestTypes.h"
#include "Quest/UKQuestRewardTypes.h"
#include "Quest/UKQuestObjectiveTypes.h"
#include "Engine/DataTable.h"
#include "DataAsset/Data/UK_ItemData.h"

// [Preset] 추가 include
#include "Quest/UKQuestPresetAsset.h"
#include "Quest/UKQuestPresetTypes.h"

#include "Quest/DataAsset/UKQuestDefinitionAsset.h"
#include "Quest/UKQuestEventParsing.h"

#include "UKQuestManagerSubsystem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UK_API UUKQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// [1] Subsystem Lifecycle

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:

	// [2] Runtime Progress
	// 런타임 진행도(메모리 상)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FName, FQuestProgress> RuntimeProgress;


	// [3] Quest 기본 API
	UFUNCTION(BlueprintCallable)
	bool StartQuest(FName QuestId);

	UFUNCTION(BlueprintCallable)
	bool CompleteQuest(FName QuestId);

	UFUNCTION(BlueprintCallable)
	bool SetQuestStep(FName QuestId, int32 NewStep);

	// ---- 이벤트 라우터(뼈대) ----
	UFUNCTION(BlueprintCallable)
	void EmitQuestEvent(FName EventId);

protected:

	// [4] Reward / ItemDataTable (기존 유지)
	// 에디터에서 할당할 아이템 정보 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "UK|Config")
	class UDataTable* ItemDataTable;

	// 데이터 테이블에서 아이템 정보를 찾아오는 헬퍼 함수
	const FUK_ItemData* GetItemData(FName ItemRowName) const;

public:
	// 퀘스트 보상을 실제로 지급하는 함수
	UFUNCTION(BlueprintCallable, Category = "UK|Quest")
	void GiveQuestReward(FName ItemRowName, int32 Amount);

protected:
	// [Reward v2] RewardId -> RewardRow(DataTable)
	UPROPERTY(EditDefaultsOnly, Category = "UK|Quest|Reward")
	TObjectPtr<UDataTable> RewardDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "UK|Quest|Reward")
	FSoftObjectPath RewardDataTablePath;

	// RewardId로 DT를 읽어 실제 지급/반영
	bool ApplyRewardById(FName RewardId, FName QuestId /*로그용*/);

protected:

	// [5] Preset
	// 프리셋 에셋을 런타임에 들고 있기
	UPROPERTY(Transient)
	TObjectPtr<UUKQuestPresetAsset> PresetAsset = nullptr;

	// 프리셋 에셋 경로(DA_QuestPresets 레퍼런스)
	//  - 초기엔 코드에서 기본값을 세팅하거나,
	//  - 나중에 DeveloperSettings로 뺄 수 있음
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Preset")
	FSoftObjectPath PresetAssetPath;

	// QuestID(Q_<Zone>_<M|S|T>_<Tag>_<NNN>)에서 Tag를 뽑아내기
	bool ParseQuestTagFromQuestId(FName QuestId, EUKQuestTag& OutTag) const;

public:
	// [6] Save/Load
	UFUNCTION(BlueprintCallable)
	bool SaveToSlot(const FString& SlotName = TEXT("UK_Save"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable)
	bool LoadFromSlot(const FString& SlotName = TEXT("UK_Save"), int32 UserIndex = 0);


	// [7] 조회
	UFUNCTION(BlueprintCallable)
	bool GetProgress(FName QuestId, FQuestProgress& OutProgress) const;

	//----------------------------------------------------------------------------------------

protected:
	// [Quest Definitions] (신규)

protected:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<const UUKQuestDefinitionAsset>> QuestDefinitions;

public:
	// 에디터/런타임에서 퀘스트 정의를 등록(DA 인스턴스 전달)
	UFUNCTION(BlueprintCallable, Category = "UK|Quest|Definition")
	bool RegisterQuestDefinition(const UUKQuestDefinitionAsset* Definition);

	UFUNCTION(BlueprintCallable, Category = "UK|Quest|Definition")
	const UUKQuestDefinitionAsset* GetQuestDefinition(FName QuestId) const;

protected:
	// [Progress Helpers] (신규)

	FName MakeCounterKey(FName QuestId, FName CounterName) const; // C.<QuestID>.<Name>
	FName MakeFlagKey(FName QuestId, FName Category) const;       // F.<QuestID>.<Category>

	int32 GetCounter(const FQuestProgress& P, FName CounterKey) const;
	void  SetCounter(FQuestProgress& P, FName CounterKey, int32 Value);
	void  AddCounter(FQuestProgress& P, FName CounterKey, int32 Delta);

	bool  HasFlag(const FQuestProgress& P, FName FlagKey) const;
	void  SetFlag(FQuestProgress& P, FName FlagKey);

	bool  IsObjectiveComplete(const FQuestProgress& P, const FUKQuestObjectiveDef& Obj, FName QuestId) const;
	void  TryAutoCompleteQuest(FName QuestId); 
	 
}; 