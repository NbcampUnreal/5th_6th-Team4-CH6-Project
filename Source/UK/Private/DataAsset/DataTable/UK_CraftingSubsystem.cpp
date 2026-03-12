#include "DataAsset/DataTable/UK_CraftingSubsystem.h"
#include "DataAsset/DataTable/UK_CraftingRecipeRow.h"
void UUK_CraftingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	static ConstructorHelpers::FObjectFinder<UDataTable> RecipeTableObj(TEXT("/Game/ItemData/DT_WeaponRecipes"));
	if (RecipeTableObj.Succeeded()) RecipeDataTable = RecipeTableObj.Object;
	
	static ConstructorHelpers::FObjectFinder<UDataTable> ItemTableObj(TEXT("/Game/ItemData/DT_ItemTableble"));
	if (ItemTableObj.Succeeded()) ItemDataTable = ItemTableObj.Object;
}

const FUK_ItemData* UUK_CraftingSubsystem::GetItemData(FName ItemId) const
{
	if (!ItemDataTable) return nullptr;
	return ItemDataTable->FindRow<FUK_ItemData>(ItemId, TEXT("CraftingSystem"));
}

bool UUK_CraftingSubsystem::TryCraftItem(FName RecipeRowName)
{
	if (!RecipeDataTable || !ItemDataTable) return false;

	// 1. 레시피 데이터 가져오기
	FUK_CraftingRecipeRow* Recipe = RecipeDataTable->FindRow<FUK_CraftingRecipeRow>(RecipeRowName, TEXT("CraftingContext"));
	if (!Recipe) return false;

	// 2. [골드 체크] (형님의 PlayerState나 CurrencySubsystem에서 가져와야 함)
	// if (GetCurrentGold() < Recipe->RequiredGold) return false;

	// 3. [재료 체크] 인벤토리에 재료가 충분한지 확인
	for (const auto& Ingredient : Recipe->RequiredItems)
	{
		FName IngredientId = Ingredient.Key;
		int32 RequiredCount = Ingredient.Value;

		// 인벤토리에서 해당 아이템 개수 확인 (형님의 인벤토리 시스템 연결부)
		// if (GetInventoryCount(IngredientId) < RequiredCount) return false;
	}

	// 4. [제작 실행] 재료/골드 차감 및 아이템 지급
	UE_LOG(LogTemp, Log, TEXT("[Crafting] %s 제작 성공!"), *Recipe->RecipeDisplayName.ToString());
    
	// SpendGold(Recipe->RequiredGold);
	// RemoveItemsFromInventory(Recipe->RequiredItems);
	// GiveItem(Recipe->TargetItemId, 1);

	return true;
}