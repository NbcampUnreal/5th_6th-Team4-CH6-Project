#pragma once

#include "CoreMinimal.h"
#include "UKDialogueTypes.generated.h"

/*
	다이어로그 json 파일에서 필요한것들을 

	1) JSON 원본 구조
	   - Choice
	   - Node
	   - Dialogue
	   - DialoguePack

	2) UI 전달 구조
	   - Choice UI Data
	   - Current Dialogue UI Data
*/



// [1] JSON Choice
// 대화중 선택지와 관련된 내용들 
USTRUCT(BlueprintType)
struct FUKDialogueChoice
{
	GENERATED_BODY()

	// UI에 표시될 선택지 문구
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	// 다음으로 이동할 NodeId
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Next;

	// 선택 시 발생시킬 QuestEvent 목록
	//emit 발사기
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Emit;

	// 선택 시 세팅할 Flag 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> SetFlags;
};



// [2] JSON Node
// 노드와 관련된 내용들
USTRUCT(BlueprintType)
struct FUKDialogueNode
{
	GENERATED_BODY()

	// N_001, N_002, N_END 등
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Id;

	// NPC_Start_GuideGirl 같은 화자 EntityID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Speaker;

	// 현재 노드 대사
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	// 선택지 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUKDialogueChoice> Choices;
};



// [3] JSON Dialogue
// 다이어로그 오픈과 관련된 내용들
USTRUCT(BlueprintType)
struct FUKDialogueData
{
	GENERATED_BODY()

	// D_Q_Start_M_DIA_001_Intro 같은 DialogueId
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DialogueId;

	// 시작 노드
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StartNode;

	// 이 대화를 시작하기 위해 필요한 플래그들
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiresFlags;

	// 실제 노드들
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUKDialogueNode> Nodes;
};



// [4] JSON Dialogue Pack
USTRUCT(BlueprintType)
struct FUKDialoguePack
{
	GENERATED_BODY()

	// JSON 파일 안의 dialogue 배열 전체
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUKDialogueData> Dialogues;
};



// [5] UI Choice Data
// UI - 선택지에서 쓰면되는것들
USTRUCT(BlueprintType)
struct FUKDialogueChoiceUIData
{
	GENERATED_BODY()

	// 선택지 인덱스
	UPROPERTY(BlueprintReadOnly)
	int32 ChoiceIndex = INDEX_NONE;

	// UI에 보여줄 선택지 문구
	UPROPERTY(BlueprintReadOnly)
	FText ChoiceText;
};



// [6] UI Current Dialogue Data
// UI에서 쓰면되는것들 (글)
USTRUCT(BlueprintType)
struct FUKCurrentDialogueUIData
{
	GENERATED_BODY()

	// 현재 대화 ID
	UPROPERTY(BlueprintReadOnly)
	FName DialogueId;

	// 현재 노드 ID
	UPROPERTY(BlueprintReadOnly)
	FName NodeId;

	// 현재 화자 ID (예: NPC_Start_GuideGirl)
	UPROPERTY(BlueprintReadOnly)
	FName SpeakerId;

	// 현재 화자 이름 (NPC DT에서 변환된 값) (예:시작마을 첫번째 npc)
	UPROPERTY(BlueprintReadOnly)
	FText SpeakerName;

	// 현재 대사 텍스트
	UPROPERTY(BlueprintReadOnly)
	FText DialogueText;

	// 현재 선택지 목록
	UPROPERTY(BlueprintReadOnly)
	TArray<FUKDialogueChoiceUIData> Choices;

	// 대화 종료 여부
	UPROPERTY(BlueprintReadOnly)
	bool bIsEnd = false;
};