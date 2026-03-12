#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/UK_CharacterBase.h"
#include "UK_ItemTableHelper.generated.h"

class UDataTable;
class UTexture2D;

USTRUCT(BlueprintType)
struct FUK_ItemTableRowView
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(BlueprintReadOnly)
	FText ItemDescription;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemIcon;

	UPROPERTY(BlueprintReadOnly)
	int32 ItemStackCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxItemStack = 1;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ItemTag;

	UPROPERTY(BlueprintReadOnly)
	bool bIsWeapon = false;

	UPROPERTY(BlueprintReadOnly)
	ECharacterAttribute WeaponAttribute = ECharacterAttribute();
};

namespace UK_ItemTableHelper
{
	bool FindItemData(const TArray<TObjectPtr<UDataTable>>& Tables, FName ItemID, FUK_ItemTableRowView& OutRow);
}