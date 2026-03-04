#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_FindTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UUK_BTTaskNode_FindTarget::UUK_BTTaskNode_FindTarget()
{
	NodeName = TEXT("Find Target");
}

EBTNodeResult::Type UUK_BTTaskNode_FindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory
)
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!Player) return EBTNodeResult::Failed;

	BB->SetValueAsObject("TargetActor", Player);

	return EBTNodeResult::Succeeded;
}