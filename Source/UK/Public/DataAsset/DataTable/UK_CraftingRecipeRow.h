#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UK_CraftingRecipeRow.generated.h"

USTRUCT(BlueprintType)
struct FUK_CraftingRecipeRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	// 결과물 아이템 ID (팀원 테이블 RowName과 매칭)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FName TargetItemId;

	// 필요 재료 (아이템ID : 개수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	TMap<FName, int32> RequiredItems;

	// 필요 골드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 RequiredGold = 0;

	// 제작법 화면 표시 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FText RecipeDisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	UTexture2D* RecipeIcon;
};
