#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_BossReset.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"
#include "BehaviorTree/BlackboardComponent.h"

UUK_BTTaskNode_BossReset::UUK_BTTaskNode_BossReset()
{
	NodeName = TEXT("Boss Reset");
}

EBTNodeResult::Type UUK_BTTaskNode_BossReset::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICtl = OwnerComp.GetAIOwner();
	if (!AICtl) return EBTNodeResult::Failed;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());
	if (!Boss) return EBTNodeResult::Failed;

	// BB 리셋 — 귀환 상태 완전 초기화
	if (UBlackboardComponent* BB = AICtl->GetBlackboardComponent())
	{
		BB->SetValueAsFloat(TEXT("SearchStartTime"), 0.f); 
		BB->ClearValue(TEXT("TargetActor"));
	}

	// 보스 상태 리셋
	Boss->ResetForReturn();

	// HP 리셋
	if (auto* MonsterAS = Boss->GetMonsterAttributeSet())
	{
		MonsterAS->SetHealth(MonsterAS->GetMaxHealth());
	}

	AUK_BossMonster_Grux* Grux = Cast<AUK_BossMonster_Grux>(Boss);
	if (Grux) Grux->UpdatePhase();

	return EBTNodeResult::Succeeded;
}
