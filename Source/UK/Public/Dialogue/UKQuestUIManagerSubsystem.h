#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dialogue/UKDialogueTypes.h"
#include "Quest/UKQuestObjectiveTypes.h"
#include "UKQuestUIManagerSubsystem.generated.h"

class UUKQuestManagerSubsystem;
class UUKDialogueSubsystem;
class UUKQuestDefinitionAsset;

UENUM(BlueprintType) //UI마커 활용용도. 추후 정리할것임
enum class EUKQuestMarkerState : uint8
{
	Hidden        UMETA(DisplayName = "Hidden"),
	InProgress    UMETA(DisplayName = "InProgress"),
	ReadyToTurnIn UMETA(DisplayName = "ReadyToTurnIn")
};

UENUM(BlueprintType)
enum class EUKQuestMarkerTargetType : uint8
{
	None            UMETA(DisplayName = "None"),
	NPC             UMETA(DisplayName = "NPC"),
	Warp            UMETA(DisplayName = "Warp"),
	MonsterSpawner  UMETA(DisplayName = "MonsterSpawner")
};

USTRUCT(BlueprintType)
struct FUKQuestMarkerRouteInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName QuestId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FName TargetId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	EUKQuestMarkerState MarkerState = EUKQuestMarkerState::Hidden;

	UPROPERTY(BlueprintReadOnly)
	EUKQuestMarkerTargetType TargetType = EUKQuestMarkerTargetType::None;
};

UCLASS(BlueprintType, Blueprintable)
class UK_API UUKQuestUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	// [1] Lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	
	// [2] Cached Subsystems
	UPROPERTY(Transient)
	TObjectPtr<UUKQuestManagerSubsystem> QuestSubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUKDialogueSubsystem> DialogueSubsystem = nullptr;

protected:
	
	// [3] Internal Helpers
	UUKQuestManagerSubsystem* GetQuestSubsystem() const;
	UUKDialogueSubsystem* GetDialogueSubsystem() const;
	const UUKQuestDefinitionAsset* GetQuestDefinitionSafe(FName QuestId) const;

	const FUKQuestObjectiveDef* FindFirstUnsatisfiedObjective(FName QuestId) const;
	bool IsWarpObjectiveTarget(const FName& TargetId) const;
	FName ResolveReportNpcId(FName QuestId) const;
	FName ResolveOfferNpcId(FName QuestId) const;
	TArray<const FUKQuestObjectiveDef*> FindUnsatisfiedObjectives(FName QuestId) const;


public:
	
	// [4] Quest UI Text Getter

	/*
		[UI 용도]
		- 퀘스트 목록 UI / 퀘스트 상세 패널의 "퀘스트 제목" 텍스트에 사용
		- 기획 담당은 QuestDefinitionAsset의 QuestTitle(또는 Title)에 넣어두면 됨
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FText GetQuestTitleText(FName QuestId) const;

	/*
		[UI 용도]
		- 퀘스트 상세 UI의 "퀘스트 설명" 텍스트에 사용
		- 기획 담당은 QuestDefinitionAsset의 QuestDescription(또는 Description)에 넣어두면 됨
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FText GetQuestDescriptionText(FName QuestId) const;

public:
	
	// [5] Quest State Bool Getter

	/*
		[UI 용도]
		- "수락" 버튼 활성/비활성
		- true면 아직 시작되지 않은 퀘스트로 간주
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	bool CanAcceptQuest(FName QuestId) const;

	/*
		[UI 용도]
		- "진행 중" 표시 여부
		- true면 이미 시작했고 아직 완료되지 않은 상태
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	bool IsQuestInProgress(FName QuestId) const;

	/*
		[UI 용도]
		- "완료됨" 배지, 완료 표시, 완료 목록 분류 등에 사용
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	bool IsQuestCompleted(FName QuestId) const;

	/*
		[UI 용도]
		- "완료 보고 가능", "목표 달성" 표시
		- true면 현재 Objective들이 모두 충족된 상태
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	bool AreObjectivesSatisfied(FName QuestId) const;

	/*
	[UI/Marker 용도]
	- 퀘스트 추적 마커 상태를 한 번에 반환
	- Hidden        : 완료했거나, 아직 진행 중이 아님
	- InProgress    : 진행 중이지만 아직 완료 보고 단계는 아님
	- ReadyToTurnIn : 진행 중이며 목표를 모두 달성해 완료 보고 가능
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	EUKQuestMarkerState GetQuestMarkerState(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	EUKQuestMarkerTargetType GetQuestMarkerTargetType(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FName GetQuestMarkerTargetId(FName QuestId, EUKQuestMarkerTargetType TargetType) const;

	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FUKQuestMarkerRouteInfo GetQuestMarkerRouteInfo(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	TArray<FUKQuestMarkerRouteInfo> GetQuestMarkerRouteInfos(FName QuestId) const;

public:
	
	// [6] Dialogue UI Getter

	/*
		[UI 용도]
		- 현재 대화 화자 이름 텍스트칸
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FText GetCurrentDialogueSpeakerName() const;

	/*
		[UI 용도]
		- 현재 대화 본문 텍스트칸
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FText GetCurrentDialogueText() const;

	/*
		[UI 용도]
		- 선택지 버튼 리스트의 텍스트 배열
		- 반환된 순서대로 버튼 0,1,2...에 연결하면 됨
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	TArray<FText> GetCurrentDialogueChoiceTexts() const;

	/*
		[UI 용도]
		- 현재 대화창 전체 데이터
		- 화자, 대사, 선택지, 종료 여부를 한 번에 받고 싶을 때 사용
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	FUKCurrentDialogueUIData GetCurrentDialogueUIData() const;

	/*
		[UI 용도]
		- 선택지 버튼 클릭 시 호출
		- ChoiceIndex는 버튼 인덱스와 동일하게 전달하면 됨
	*/
	UFUNCTION(BlueprintCallable, Category = "UK|QuestUI")
	bool SelectDialogueChoice(int32 ChoiceIndex);
};