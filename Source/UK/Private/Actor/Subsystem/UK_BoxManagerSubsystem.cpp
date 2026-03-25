#include "Actor/Subsystem/UK_BoxManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UUK_BoxManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (UUK_InGameSave* LoadGame = Cast<UUK_InGameSave>(UGameplayStatics::LoadGameFromSlot(TEXT("BoxSaveSlot"), 0)))
	{
		for (const FName& ID : LoadGame->OpenedChestIDs)
		{
			OpenedChestIDs.Add(ID);
		}
	}
}

bool UUK_BoxManagerSubsystem::ShouldSpawnChest(FName ChestID)
{
	return !OpenedChestIDs.Contains(ChestID);
}

void UUK_BoxManagerSubsystem::RegisterOpenedChest(FName ChestID)
{
	OpenedChestIDs.Add(ChestID);
	SaveToDisk();
}

void UUK_BoxManagerSubsystem::SaveToDisk()
{
	UUK_InGameSave* SaveGame = Cast<UUK_InGameSave>(UGameplayStatics::LoadGameFromSlot(TEXT("BoxSaveSlot"), 0));
	if (!SaveGame)
	{
		SaveGame = Cast<UUK_InGameSave>(UGameplayStatics::CreateSaveGameObject(UUK_InGameSave::StaticClass()));
	}
	if (SaveGame)
	{
		SaveGame->OpenedChestIDs = OpenedChestIDs.Array();
		bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("BoxSaveSlot"), 0);
       
		if (bSuccess)
		{
		}
	}
}
