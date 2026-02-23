// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/BehaviorTree/UK_BTService_SetWalkSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UUK_BTService_SetWalkSpeed::UUK_BTService_SetWalkSpeed()
{
	NodeName = "Set Walk Speed";
	// 주기적 Tick 불필요 - 진입/이탈 시에만 처리
	Interval = 0.f;
	RandomDeviation = 0.f;
}

void UUK_BTService_SetWalkSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return;

	// 현재 속도 캐싱
	CachedOriginalSpeed = MoveComp->MaxWalkSpeed;

	// 배회 속도 적용
	MoveComp->MaxWalkSpeed = WanderSpeed;
}

void UUK_BTService_SetWalkSpeed::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return;

	// 속도 복원 (OriginalSpeed가 설정됐으면 그걸 사용, 아니면 캐싱값)
	MoveComp->MaxWalkSpeed = (OriginalSpeed > 0.f) ? OriginalSpeed : CachedOriginalSpeed;
}