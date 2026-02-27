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

#pragma region Initialization
public:
	UUK_BTService_DetectPlayer_Peaceful();
#pragma endregion

#pragma region Player Detection
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion

#pragma region Blackboard Keys
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;
#pragma endregion

#pragma region Settings
	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 1000.0f;
#pragma endregion
};