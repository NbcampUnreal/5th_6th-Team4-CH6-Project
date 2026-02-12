#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_DetectPlayer_Peaceful.generated.h"

/**
 * 평화로운 몬스터용 플레이어 감지 서비스
 * 적대적일 때 Aggressor를 우선 타겟으로 설정
 */
UCLASS()
class UK_API UUK_BTService_DetectPlayer_Peaceful : public UBTService
{
	GENERATED_BODY()

public:
	UUK_BTService_DetectPlayer_Peaceful();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 1000.0f;
};