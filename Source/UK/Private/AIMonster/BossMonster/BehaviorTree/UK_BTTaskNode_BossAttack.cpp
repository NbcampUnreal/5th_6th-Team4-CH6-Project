#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_BossAttack.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/BossMonster/UK_BossAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

UUK_BTTaskNode_BossAttack::UUK_BTTaskNode_BossAttack()
{
	NodeName = TEXT("Boss Attack");
	bNotifyTick = true;
	bCreateNodeInstance = true;
	AttackDuration = 5.0f;
	CurrentTime = 0.f;
}

EBTNodeResult::Type UUK_BTTaskNode_BossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CurrentTime = 0.f;
 
	AAIController* AICtl = OwnerComp.GetAIOwner();
	if (!AICtl) return EBTNodeResult::Failed;
 
	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());
	if (!Boss || Boss->bIsAttacking) return EBTNodeResult::Failed;
 
	AICtl->StopMovement();
	if (!Boss->PlayRandomAttackMontage()) return EBTNodeResult::Failed;

	CachedBoss = Boss;
	CachedOwnerComp = &OwnerComp;
	
	TWeakObjectPtr<UBehaviorTreeComponent> WeakComp(&OwnerComp);
	Boss->OnAttackFinished.BindLambda([this, WeakComp]()
	{
		if (WeakComp.IsValid())
			FinishLatentTask(*WeakComp.Get(), EBTNodeResult::Succeeded);
	});
 
	return EBTNodeResult::InProgress;
}

void UUK_BTTaskNode_BossAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 폴링 로직 제거 — 델리게이트가 처리
	// 타임아웃 안전장치만 유지
	CurrentTime += DeltaSeconds;
	if (CurrentTime >= AttackDuration)
	{
		CurrentTime = 0.f;
		if (CachedBoss) CachedBoss->OnAttackFinished.Unbind();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UUK_BTTaskNode_BossAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	CurrentTime = 0.f;
	if (CachedBoss)
	{
		CachedBoss->OnAttackFinished.Unbind();
		CachedBoss = nullptr;
	}
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
