#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_CheckChaseDistance.generated.h"

/**
 * 추격 중 플레이어와의 거리를 체크하여 너무 멀어지면 타겟 클리어
 * 일반 몬스터와 평화 몬스터 모두 사용 가능
 */
UCLASS()
class UK_API UUK_BTService_CheckChaseDistance : public UBTService
{
	GENERATED_BODY()

public:
	UUK_BTService_CheckChaseDistance();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	// 추격 포기 거리 (이 거리를 벗어나면 추격 중단)
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxChaseDistance = 1500.0f;
};