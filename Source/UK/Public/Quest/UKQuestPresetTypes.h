#pragma once

#include "CoreMinimal.h"
#include "UKQuestPresetTypes.generated.h"

// QuestID에는 Tag가 이미 들어가지만,
// 프리셋 선택/적용은 enum이 훨씬 안전합니다.
UENUM(BlueprintType)
enum class EUKQuestTag : uint8
{
	WPN UMETA(DisplayName = "WPN (Weapon)"),
	SKL UMETA(DisplayName = "SKL (Skill)"),
	CFT UMETA(DisplayName = "CFT (Craft)"),
	EXP UMETA(DisplayName = "EXP (Explore)"),
	HNT UMETA(DisplayName = "HNT (Hunt)"),
	DLV UMETA(DisplayName = "DLV (Deliver)"),
	DIA UMETA(DisplayName = "DIA (Dialogue)"),
	ESC UMETA(DisplayName = "ESC (Escort)"),
	DEF UMETA(DisplayName = "DEF (Defense)"),
	COL UMETA(DisplayName = "COL (Collect)"),
	BOS UMETA(DisplayName = "BOS (Boss)"),
	DNG UMETA(DisplayName = "DNG (Dungeon)"),
};

USTRUCT(BlueprintType)
struct FUKQuestPresetData
{
	GENERATED_BODY()

	// ---- 공통(대부분의 퀘스트가 공유할 기본값) ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DefaultStartStep = 0;

	// Accepted/Completed/Failed 같은 기본 FlagCategory를 “자동 생성”할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAutoInitCommonFlags = true;

	// ---- 태그별로 자주 쓰는 기본 Counter/Flag 템플릿(키는 Name만 저장) ----
	// 실제 저장 키는: C.<QuestID>.<Name>, F.<QuestID>.<Category> 규칙으로 생성한다고 가정
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> DefaultCounterNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> DefaultFlagCategories;

	// ---- (선택) UI/디버그용 기본 메타 ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText PresetDisplayName;
};