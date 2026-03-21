#pragma once

#include "CoreMinimal.h"
#include "UKQuestConditionTypes.generated.h"

/*
	[Condition Operand]
	- Clause가 어떤 종류의 값을 읽을지 정의
	- 1차 구현:
		QuestFlag / QuestCounter / QuestState 우선 지원
	- GlobalFlag / GlobalCounter / Custom은 구조만 열어두고
	  실제 평가는 차후 확장 가능
*/
UENUM(BlueprintType)
enum class EUKConditionOperandType : uint8
{
	QuestFlag      UMETA(DisplayName = "QuestFlag"),
	QuestCounter   UMETA(DisplayName = "QuestCounter"),
	GlobalFlag     UMETA(DisplayName = "GlobalFlag"),
	GlobalCounter  UMETA(DisplayName = "GlobalCounter"),
	QuestState     UMETA(DisplayName = "QuestState"),
	Custom         UMETA(DisplayName = "Custom")
};

/*
	[Quest State Field]
	- QuestState 타입에서 어떤 상태를 볼지 정의
*/
UENUM(BlueprintType)
enum class EUKQuestStateField : uint8
{
	Accepted   UMETA(DisplayName = "Accepted"),
	Completed  UMETA(DisplayName = "Completed"),
	Failed     UMETA(DisplayName = "Failed")
};

/*
	[Compare Operator]
	- Clause 비교 연산자
*/
UENUM(BlueprintType)
enum class EUKConditionCompareOp : uint8
{
	Equal           UMETA(DisplayName = "=="),
	NotEqual        UMETA(DisplayName = "!="),
	Greater         UMETA(DisplayName = ">"),
	GreaterOrEqual  UMETA(DisplayName = ">="),
	Less            UMETA(DisplayName = "<"),
	LessOrEqual     UMETA(DisplayName = "<=")
};

//[Group Operator] - ConditionID 내부 Clause 묶는 방식
UENUM(BlueprintType)
enum class EUKConditionGroupOp : uint8
{
	AllOf UMETA(DisplayName = "AND"),
	AnyOf UMETA(DisplayName = "OR")
};

/*
	[Condition Clause]
	- 조건 한 줄
	- 예:
		OperandType = QuestFlag
		KeyName      = Accepted
		CompareOp    = Equal
		BoolValue    = false
*/
USTRUCT(BlueprintType)
struct FUKQuestConditionClause
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EUKConditionOperandType OperandType = EUKConditionOperandType::QuestFlag;

	//[Quest 전용] - QuestFlag/QuestCounter/QuestState 에서 비워두면 OwnerQuestId 사용
	// 다른 퀘스트를 참조하려면 명시적으로 입력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FName TargetQuestId = NAME_None;

	/*
		[KeyName]
		- QuestFlag  : Category 이름 (예: Accepted, Completed, GateOpened)
		- QuestCounter: CounterName (예: CollectGear)
		- QuestState : 보통 비워두고 StateField 사용
		- GlobalFlag / GlobalCounter: 전역 키 이름
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FName KeyName = NAME_None;

	//[QuestState 전용]
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EUKQuestStateField StateField = EUKQuestStateField::Accepted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EUKConditionCompareOp CompareOp = EUKConditionCompareOp::Equal;

	//[비교값] - Bool 비교용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	bool BoolValue = true;

	//[비교값]- Counter 비교용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	int32 IntValue = 0;


	// [선택]- 나중 확장용 문자열/이름 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FName ExtraName = NAME_None;
};

// 추후 확장 예정:
// CharacterLevel
// CharacterStat
// InventoryItemCount