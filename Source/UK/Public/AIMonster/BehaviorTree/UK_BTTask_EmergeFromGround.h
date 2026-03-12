#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_EmergeFromGround.generated.h"

/**
 * 땅에서 나오는 몽타주를 재생하고 완료 대기.
 * 몽타주 없으면 즉시 Succeeded (안전 처리).
 */
UCLASS()
class UK_API UUK_BTTask_EmergeFromGround : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_EmergeFromGround();
#pragma endregion

#pragma region Task Interface
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Internal
private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void CleanupMontageDelegate(UAnimInstance* AnimInst, UAnimMontage* Montage);
#pragma endregion
};