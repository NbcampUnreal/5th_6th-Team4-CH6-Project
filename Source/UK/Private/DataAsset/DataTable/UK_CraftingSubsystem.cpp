#include "DataAsset/DataTable/UK_CraftingSubsystem.h"
#include "DataAsset/DataTable/UK_CraftingRecipeRow.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "Quest/UKQuestManagerSubsystem.h"

void UUK_CraftingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RecipeDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ItemData/CraftingWeaponRecipe/CraftWeapon_Recipe")));
    
	if (RecipeDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("RecipeDataTable 로드 성공!"));
	}

	ItemDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ItemData/DT_ItemTableble")));
    
	if (ItemDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("ItemDataTable 로드 성공!"));
	}
}

const FUK_ItemData* UUK_CraftingSubsystem::GetItemData(FName ItemId) const
{
	if (!ItemDataTable) return nullptr;
	return ItemDataTable->FindRow<FUK_ItemData>(ItemId, TEXT("CraftingSystem"));
}

bool UUK_CraftingSubsystem::TryCraftItem(FName RecipeRowName)
{
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC || !PC->GetPawn()) return false;
    
	UUK_InventoryComponent* PlayerInv = PC->GetPawn()->FindComponentByClass<UUK_InventoryComponent>();
	if (!PlayerInv) return false;
    
	// 인벤토리나 레시피 테이블이 없으면 진행 불가
	if (!PlayerInv || !RecipeDataTable) return false;

	//레시피 데이터 찾기
	FUK_CraftingRecipeRow* Recipe = RecipeDataTable->FindRow<FUK_CraftingRecipeRow>(RecipeRowName, TEXT("CraftingContext"));
	if (!Recipe) return false;

	//골드 체크
	if (PlayerInv->GetGold() < Recipe->RequiredGold) 
	{
		UE_LOG(LogTemp, Warning, TEXT("골드가 부족합니다!"));
		return false;
	}

	//재료 체크
	for (const auto& Ingredient : Recipe->RequiredItems)
	{
		if (PlayerInv->GetItemTotalQuantity(Ingredient.Key) < Ingredient.Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s 재료가 부족합니다!"), *Ingredient.Key.ToString());
			return false;
		}
	}

	//재료 소모
	PlayerInv->subtractionGold(Recipe->RequiredGold);
	for (const auto& Ingredient : Recipe->RequiredItems)
	{
		PlayerInv->RemoveItem(Ingredient.Key, Ingredient.Value);
	}

	//결과물 지급
	PlayerInv->AddItem(Recipe->TargetItemId, 1);
    
	// 임시 퀘스트 권한 부여
	if ( UGameInstance* GI = GetGameInstance() )
	{
		if ( UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
		{
			QuestSys->EmitQuestEvent(FName(TEXT("QuestEvent.Custom.CraftCompleted")));
			UE_LOG(LogTemp, Warning, TEXT("[Craft][Quest] Emit CraftCompleted. Actor=%s"), *GetName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s 제작 완료!"), *Recipe->RecipeDisplayName.ToString());
	return true;
}

TArray<FName> UUK_CraftingSubsystem::GetAllRecipeRowNames() const
{
	if (!RecipeDataTable) return TArray<FName>();

	return RecipeDataTable->GetRowNames();
}

TMap<FName, FText> UUK_CraftingSubsystem::GetAllRecipeDisplayNames() const
{
	TMap<FName, FText> RecipeMap;
	if (!RecipeDataTable) return RecipeMap;
	
	TArray<FName> RowNames = RecipeDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FUK_CraftingRecipeRow* Row = RecipeDataTable->FindRow<FUK_CraftingRecipeRow>(RowName, TEXT(""));
		if (Row)
		{
			RecipeMap.Add(RowName, Row->RecipeDisplayName);
		}
	}

	return RecipeMap;
}