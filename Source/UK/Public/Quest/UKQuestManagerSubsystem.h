#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UKQuestTypes.h"
#include "UKQuestManagerSubsystem.generated.h"

UCLASS()
class UK_API UUKQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 런타임 진행도(메모리 상)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FName, FQuestProgress> RuntimeProgress;

	// ---- Quest 기본 API ----
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
	// 에디터에서 할당할 아이템 정보 데이터 테이블 
	UPROPERTY(EditDefaultsOnly, Category = "UK|Config")
	class UDataTable* ItemDataTable;

	// 데이터 테이블에서 아이템 정보를 찾아오는 헬퍼 함수 
	struct FUK_ItemData* GetItemData(FName ItemRowName);

public:
	// 퀘스트 보상을 실제로 지급하는 함수 
	UFUNCTION(BlueprintCallable, Category = "UK|Quest")
	void GiveQuestReward(FName ItemRowName, int32 Amount);

	// ---- Save/Load ----
	UFUNCTION(BlueprintCallable)
	bool SaveToSlot(const FString& SlotName = TEXT("UK_Save"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable)
	bool LoadFromSlot(const FString& SlotName = TEXT("UK_Save"), int32 UserIndex = 0);

	// ---- 조회 ----
	UFUNCTION(BlueprintCallable)
	bool GetProgress(FName QuestId, FQuestProgress& OutProgress) const;
};
