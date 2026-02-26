#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_DetectPlayer.generated.h"

UCLASS()
class UK_API UUK_BTService_DetectPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UUK_BTService_DetectPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 800.0f;

	/**
	 * 복귀 완료 판정 거리 — 스폰에서 이 거리 이내에 들어오면 재감지 허용
	 * ReturnToSpawn의 AcceptanceRadius 와 비슷하게 설정 권장 (기본 200)
	 */
	UPROPERTY(EditAnywhere, Category = "AI")
	float ReturnDistanceThreshold = 200.0f;

private:
	struct FDetectPlayerMemory
	{
		bool bHadTarget  = false;  // 직전 틱에 타겟이 있었는지
		bool bReturning  = false;  // 복귀 중 플래그 (재감지 차단)
	};
};