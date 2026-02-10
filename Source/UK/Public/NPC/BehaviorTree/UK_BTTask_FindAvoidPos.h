
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_FindAvoidPos.generated.h"

UCLASS()
class UK_API UUK_BTTask_FindAvoidPos : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UUK_BTTask_FindAvoidPos();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AvoidDistance = 400.f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector AvoidLocationKey;
};
