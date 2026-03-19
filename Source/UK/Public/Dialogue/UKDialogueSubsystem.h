#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dialogue/UKDialogueTypes.h"
#include "UKDialogueSubsystem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UK_API UUKDialogueSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// =========================
	// [1] Lifecycle
	// =========================
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// =========================
	// [2] Load / Start / End
	// =========================

	// JSON 팩 파일 로드 (예: D.Start.M.001-005.json)
	UFUNCTION(BlueprintCallable, Category = "UK|Dialogue")
	bool LoadDialoguePack(const FString& PackFileName);

	// 특정 DialogueId로 대화 시작
	UFUNCTION(BlueprintCallable, Category = "UK|Dialogue")
	bool StartDialogue(const FString& PackFileName, FName DialogueId);

	// 현재 대화 종료
	UFUNCTION(BlueprintCallable, Category = "UK|Dialogue")
	void EndDialogue();

	// 대화 시작 가능 여부(requiresFlags 검사)
	UFUNCTION(BlueprintCallable, Category = "UK|Dialogue")
	bool CanStartDialogue(FName DialogueId) const;

public:
	// =========================
	// [3] Current State Getter
	// =========================

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	FName GetCurrentDialogueId() const { return CurrentDialogueId; }

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	FName GetCurrentNodeId() const { return CurrentNodeId; }

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	FName GetCurrentSpeakerID() const;

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	FText GetCurrentSpeakerName() const;

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	FText GetCurrentDialogueText() const;

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	TArray<FText> GetCurrentChoiceTexts() const;

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	int32 GetCurrentChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	bool IsDialogueEnd() const;

	// UI 담당이 한 번에 받기 좋도록 묶어서 제공
	UFUNCTION(BlueprintPure, Category = "UK|Dialogue")
	FUKCurrentDialogueUIData GetCurrentDialogueUIData() const;

public:
	// =========================
	// [4] Progress
	// =========================

	// 선택지 선택
	UFUNCTION(BlueprintCallable, Category = "UK|Dialogue")
	bool SelectChoice(int32 ChoiceIndex);

protected:
	// =========================
	// [5] Internal State
	// =========================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UK|Dialogue")
	FString CurrentPackFileName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UK|Dialogue")
	FUKDialoguePack LoadedPack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UK|Dialogue")
	bool bPackLoaded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UK|Dialogue")
	FName CurrentDialogueId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UK|Dialogue")
	FName CurrentNodeId;

protected:
	// =========================
	// [6] Helpers
	// =========================

	const FUKDialogueData* FindDialogueById(FName DialogueId) const;
	const FUKDialogueNode* FindNodeById(const FUKDialogueData& Dialogue, FName NodeId) const;
	const FUKDialogueNode* GetCurrentNode() const;
	const FUKDialogueData* GetCurrentDialogue() const;

	// JSON 파일 절대 경로 만들기
	FString BuildDialogueJsonAbsolutePath(const FString& PackFileName) const;

	// JSON 파싱
	bool ParseDialoguePackFromJson(const FString& JsonString, FUKDialoguePack& OutPack) const;

	// 퀘스트 시스템 연동
	void ApplyEmitEvents(const TArray<FName>& EventIds) const;
	void ApplySetFlagsFallback(const TArray<FName>& FlagKeys) const;

	// requiresFlags / setFlags 지원용
	bool HasFlagKey(FName FlagKey) const;
};