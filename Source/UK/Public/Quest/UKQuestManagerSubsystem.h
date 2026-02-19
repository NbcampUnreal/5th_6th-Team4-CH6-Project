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

	// ---- Save/Load ----
	UFUNCTION(BlueprintCallable)
	bool SaveToSlot(const FString& SlotName = TEXT("UK_Save"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable)
	bool LoadFromSlot(const FString& SlotName = TEXT("UK_Save"), int32 UserIndex = 0);

	// ---- 조회 ----
	UFUNCTION(BlueprintCallable)
	bool GetProgress(FName QuestId, FQuestProgress& OutProgress) const;
};
