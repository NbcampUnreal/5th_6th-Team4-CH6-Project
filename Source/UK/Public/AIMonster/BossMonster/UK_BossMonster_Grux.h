#pragma once

#include "CoreMinimal.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "UK_BossMonster_Grux.generated.h"

UCLASS()
class UK_API AUK_BossMonster_Grux : public AUK_BossMonsterBase
{
	GENERATED_BODY()
	
public:
	AUK_BossMonster_Grux();

protected:
	virtual void BeginPlay() override;

	virtual void OnPhaseChanged(EBossPhase NewPhase);
};
