#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTaskNode_BossAttack.generated.h"

UCLASS()
class UK_API UUK_BTTaskNode_BossAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUK_BTTaskNode_BossAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
	
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds) override;

	UPROPERTY(EditAnywhere)
	float AttackDuration;

	float CurrentTime;
	
private:
	UPROPERTY()
	TObjectPtr<AUK_BossMonsterBase> CachedBoss;
    
	FDelegateHandle AttackFinishedHandle;
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
};
