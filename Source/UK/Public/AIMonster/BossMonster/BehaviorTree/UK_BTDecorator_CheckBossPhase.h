#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "GameplayTagContainer.h"
#include "UK_BTDecorator_CheckBossPhase.generated.h"

UCLASS()
class UK_API UUK_BTDecorator_CheckBossPhase : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UUK_BTDecorator_CheckBossPhase();

protected:

	UPROPERTY(EditAnywhere, Category="Boss")
	FGameplayTag RequiredPhaseTag;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const override;
};
