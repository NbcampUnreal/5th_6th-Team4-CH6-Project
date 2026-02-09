#include "NPC/BehaviorTree/UK_BTTask_FindAvoidPos.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UUK_BTTask_FindAvoidPos::UUK_BTTask_FindAvoidPos()
{
	NodeName = "Find Avoid Position";
}

EBTNodeResult::Type UUK_BTTask_FindAvoidPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if ( !AI ) return EBTNodeResult::Failed;

	APawn* NPC = AI->GetPawn();
	if ( !NPC ) return EBTNodeResult::Failed;

	ACharacter* Player =
		UGameplayStatics::GetPlayerCharacter(NPC->GetWorld(), 0);

	if ( !Player ) return EBTNodeResult::Failed;

	FVector Dir =
		( NPC->GetActorLocation() - Player->GetActorLocation() ).GetSafeNormal();

	FVector AvoidPos =
		NPC->GetActorLocation() + Dir * AvoidDistance;

	OwnerComp.GetBlackboardComponent()->
		SetValueAsVector(AvoidLocationKey.SelectedKeyName, AvoidPos);

	return EBTNodeResult::Succeeded;
}