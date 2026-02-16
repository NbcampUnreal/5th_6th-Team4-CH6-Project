#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_DetectPlayer.generated.h"

/**
 * 플레이어 감지 및 추적 거리 체크 서비스
 * - 가장 가까운 플레이어를 찾아 Blackboard에 설정
 */
UCLASS()
class UK_API UUK_BTService_DetectPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UUK_BTService_DetectPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 800.0f;
};