#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_ElitePhaseCheck.generated.h"

/**
 * 엘리트 몬스터 특수 공격 쿨다운 상태를 블랙보드에 동기화하는 서비스
 * ■ bCanSpecialAttack — 특수 공격 가능 여부 (쿨다운 체크)
 */
UCLASS()
class UK_API UUK_BTService_ElitePhaseCheck : public UBTService
{
	GENERATED_BODY()

public:
	UUK_BTService_ElitePhaseCheck();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 특수 공격 가능 여부를 저장할 블랙보드 키 (Bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanSpecialAttackKey;
};