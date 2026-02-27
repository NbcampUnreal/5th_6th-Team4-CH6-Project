#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_CheckPlayerProximity.generated.h"

/**
 * 플레이어가 경계 거리 안에 들어왔는지 체크하는 서비스
 * 평화로운 몬스터용
 */
UCLASS()
class UK_API UUK_BTService_CheckPlayerProximity : public UBTService
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTService_CheckPlayerProximity();
#pragma endregion

#pragma region Proximity Check
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion

#pragma region Blackboard Keys
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsPlayerCloseKey;  // bool 키

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;
#pragma endregion
};