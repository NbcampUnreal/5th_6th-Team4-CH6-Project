// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_FindWanderLocation.generated.h"

/**
 * 평화 몬스터용 배회 위치 탐색 태스크
 * - 스폰 위치 기준 WanderRadius 내 랜덤 위치 반환
 * - PatrolRadius보다 작은 반경으로 자연스럽게 배회
 */
UCLASS()
class UK_API UUK_BTTask_FindWanderLocation : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_FindWanderLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Wander Location Search
protected:
	/** 배회 목적지를 저장할 블랙보드 키 (Vector) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WanderLocationKey;

	/** 스폰 위치 기준으로 배회할 최대 반경 */
	UPROPERTY(EditAnywhere, Category = "Wander", meta = (ClampMin = "100.0"))
	float WanderRadius = 500.0f;

	/**
	 * true 이면 몬스터 에셋의 PatrolRadius 기반 반경 사용
	 * 멀리 나가면 스폰 위치로 되돌아오도록 유도
	 */
	UPROPERTY(EditAnywhere, Category = "Wander")
	bool bUseMonsterRadiusFromAsset = true;
#pragma endregion
};