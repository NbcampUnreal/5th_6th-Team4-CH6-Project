#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Systems/Data/UK_InGameSave.h"
#include "UK_BoxManagerSubsystem.generated.h"

UCLASS()
class UK_API UUK_BoxManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	bool ShouldSpawnChest(FName ChestID);
	
	void RegisterOpenedChest(FName ChestID);
	
private:
	TSet<FName> OpenedChestIDs;
	
	void SaveToDisk();
};
