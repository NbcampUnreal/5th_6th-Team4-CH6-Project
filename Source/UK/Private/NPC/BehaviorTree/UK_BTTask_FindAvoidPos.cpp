#include "NPC/BehaviorTree/UK_BTTask_FindAvoidPos.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"

UUK_BTTask_FindAvoidPos::UUK_BTTask_FindAvoidPos()
{
	NodeName = "Find Avoid Position";
}

EBTNodeResult::Type UUK_BTTask_FindAvoidPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if ( !AI ) return EBTNodeResult::Failed;

	APawn* NPC = AI->GetPawn();
	if (!NPC) return EBTNodeResult::Failed;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(NPC->GetWorld(), 0);
	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(NPC->GetWorld());
	
	if (!Nav) return EBTNodeResult::Failed;
	if (!Player) return EBTNodeResult::Failed;

	FVector MyLoc = NPC->GetActorLocation();
	FVector CombinedDir = (MyLoc - Player->GetActorLocation()).GetSafeNormal();
	for (TActorIterator<APawn> It(NPC->GetWorld()); It; ++It)
	{
		APawn* OtherNPC = *It;
		if (OtherNPC && OtherNPC != NPC && OtherNPC->IsA(NPC->GetClass()))
		{
			FVector ToMe = MyLoc - OtherNPC->GetActorLocation();
			float Distance = ToMe.Size();
			if (Distance < AvoidDistance && Distance > 0.1f)
			{
				CombinedDir += ToMe.GetSafeNormal() * (1.0f - (Distance / AvoidDistance));
			}
		}
	}
	
	CombinedDir = CombinedDir.GetSafeNormal();
	FVector AvoidPos = MyLoc + CombinedDir * AvoidDistance;
    
	FNavLocation ResultPos;
	FVector QueryExtent(500.f, 500.f, 500.f); 
    
	if (Nav->ProjectPointToNavigation(AvoidPos, ResultPos, QueryExtent))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(AvoidLocationKey.SelectedKeyName, ResultPos.Location);
		return EBTNodeResult::Succeeded;
	}
    
	if (Nav->GetRandomReachablePointInRadius(MyLoc, AvoidDistance, ResultPos))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(AvoidLocationKey.SelectedKeyName, ResultPos.Location);
		return EBTNodeResult::Succeeded;
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(AvoidLocationKey.SelectedKeyName, AvoidPos);
	return EBTNodeResult::Succeeded;
}