// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_SetWalkSpeed.generated.h"

/**
 * 배회 중 이동 속도를 느리게 설정하는 서비스
 * - 노드 진입 시 WanderSpeed 적용, 이탈 시 OriginalSpeed 복원
 */
UCLASS()
class UK_API UUK_BTService_SetWalkSpeed : public UBTService
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTService_SetWalkSpeed();
#pragma endregion

#pragma region Speed Management
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Settings
	/** 배회 시 적용할 걷기 속도 */
	UPROPERTY(EditAnywhere, Category = "Speed")
	float WanderSpeed = 200.0f;

	/** 복원할 원래 속도 (0이면 자동으로 캐릭터 기본값 사용) */
	UPROPERTY(EditAnywhere, Category = "Speed")
	float OriginalSpeed = 0.0f;
#pragma endregion

#pragma region Private
private:
	float CachedOriginalSpeed = 0.f;
#pragma endregion
};