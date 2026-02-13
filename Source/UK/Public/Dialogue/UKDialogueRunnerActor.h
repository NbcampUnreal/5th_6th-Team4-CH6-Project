#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dialogue/UKDialogueTypes.h"
#include "UKDialogueRunnerActor.generated.h"

UCLASS()
class UK_API AUKDialogueRunnerActor : public AActor
{
	GENERATED_BODY()

public:
	AUKDialogueRunnerActor();

	// 트리거에서 호출할 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartDialogueById(const FString& DialogueId);

	// 선택지 1~9 선택(테스트용)
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ChooseIndex(int32 ChoiceIndex);

	// 현재 대화 진행중인지
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsRunning() const { return bRunning; }

protected:
	virtual void BeginPlay() override;

	// Content 기준 상대 경로
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FString RelativeJsonPath = TEXT("Dialogue/quest_test/Dialogues_Test_v1.json");

	// 자동 시작(원하면 true)
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	bool bAutoStartOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FString AutoStartDialogueId = TEXT("D_TestQuest_Handsome_001_v1");

private:
	// 로드된 전체 데이터
	FUKDialogueRoot Root;

	// 현재 실행 상태
	const FUKDialogueDef* ActiveDialogue = nullptr;
	const FUKDialogueNode* ActiveNode = nullptr;

	bool bLoaded = false;
	bool bRunning = false;

	bool LoadJsonIfNeeded();
	bool CheckRequiresFlags(const FUKDialogueDef& D) const;

	const FUKDialogueNode* FindNodeById(const FString& NodeId) const;

	// Node 진입 시 효과(emit / setFlags)
	void ApplyNodeSideEffects(const FUKDialogueNode& Node);

	// 로그 출력(임시 UI)
	void PrintCurrentNode() const;

	// 플래그 저장/조회(테스트 편의: QuestManager의 RuntimeProgress.Flags를 “공용키”로 사용)
	bool HasFlag(const FName Flag) const;
	void SetFlag(const FName Flag);
};