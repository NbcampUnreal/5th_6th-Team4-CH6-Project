#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UK_SaveInterface.generated.h"

UINTERFACE(MinimalAPI)
class UUK_SaveInterface : public UInterface
{
	GENERATED_BODY()
};

class UK_API IUK_SaveInterface
{
	GENERATED_BODY()
	
public:
	virtual void OnSaveGame(class UUK_InGameSave* SaveGameObject) = 0;
	virtual void OnLoadGame(class UUK_InGameSave* SaveGameObject) = 0;
};
