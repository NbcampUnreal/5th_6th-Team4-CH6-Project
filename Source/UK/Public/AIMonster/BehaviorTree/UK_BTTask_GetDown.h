#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_GetDown.generated.h"

UCLASS()
class UK_API UUK_BTTask_GetDown : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUK_BTTask_GetDown();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};