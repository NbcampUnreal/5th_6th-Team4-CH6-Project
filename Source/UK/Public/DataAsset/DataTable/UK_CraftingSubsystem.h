#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UK_CraftingSubsystem.generated.h"

struct FUK_ItemData;

UCLASS()
class UK_API UUK_CraftingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//제작 실행용 함수로 사용 예정
	UFUNCTION(BlueprintCallable, Category = "UK|Crafting")
	bool TryCraftItem(FName RecipeRowName);

	//특정아이템의 상세정보 가져오기 - 아이템 데이터 테이블읽어오는용도
	const FUK_ItemData* GetItemData(FName ItemId) const;

protected:
	//할당해야 하는 데이터 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> RecipeDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> ItemDataTable;
};
