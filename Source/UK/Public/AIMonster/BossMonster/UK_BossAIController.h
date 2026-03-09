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
	TObjectPtr<AUK_BossMonsterBase> BossPtr;
	
	UFUNCTION()
	void HandlePhaseChanged(const FGameplayTag& NewPhaseTag);
	
	const FName BB_BossPhase = TEXT("BossPhase");
	const FName BB_TargetActor = TEXT("TargetActor");
};
