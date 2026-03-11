#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_DetectPlayer.generated.h"

/**
 * 플레이어 감지 서비스 (일반 + Peaceful 통합)
 * 
 * bPeacefulMode = false (기본):
 *   - PendingTarget 사용
 *   - 복귀 상태 관리 (bReturning)
 *   - 추격 한계 체크
 * 
 * bPeacefulMode = true:
 *   - Aggressor 우선 타겟
 *   - 단순 감지만 수행
 */
UCLASS()
class UK_API UUK_BTService_DetectPlayer : public UBTService
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTService_DetectPlayer();
#pragma endregion

#pragma region Player Detection
protected:
	virtual void   TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
#pragma endregion

#pragma region Configuration
public:
	// ── 모드 설정 ────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Detection Mode", meta = (DisplayName = "Peaceful Mode"))
	bool bPeacefulMode = false;

	// ── Blackboard Keys ──────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", 
	          meta = (EditCondition = "!bPeacefulMode", EditConditionHides))
	FBlackboardKeySelector PendingTargetKey;

	// ── Detection Settings ───────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "100.0"))
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "AI", 
	          meta = (EditCondition = "!bPeacefulMode", EditConditionHides, ClampMin = "50.0"))
	float ReturnDistanceThreshold = 200.0f;
#pragma endregion

#pragma region Private Methods
private:
	void TickNormalMode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	void TickPeacefulMode(UBehaviorTreeComponent& OwnerComp);
	
	AActor* FindClosestPlayer(const FVector& Location, float MaxRadius, UWorld* World);
	AActor* TryGetPerceptionTarget(AAIController* AICon);
#pragma endregion

#pragma region Memory
private:
	struct FDetectPlayerMemory
	{
		bool bHadTarget = false; 
		bool bReturning = false;  
	};
#pragma endregion
};