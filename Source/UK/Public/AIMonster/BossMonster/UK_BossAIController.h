#pragma once

#include "CoreMinimal.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "UK_BossAIController.generated.h"

class AUK_BossMonsterBase;

UCLASS()
class UK_API AUK_BossAIController : public AUK_AiMonsterCtl
{
	GENERATED_BODY()
	
public:
	AUK_BossAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY()
	AUK_BossMonsterBase* Boss;

	void SyncBossPhaseToBB();

	FTimerHandle PhaseSyncTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float PhaseSyncInterval = 0.25f;

};
