#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UKGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS()
class AUKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AUKGameMode();
};