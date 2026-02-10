#include "NPC/BehaviorTree/BTDecorator_IsPlayerTooClose.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "Character/UK_CharacterBase.h"

UBTDecorator_IsPlayerTooClose::UBTDecorator_IsPlayerTooClose()
{
	NodeName = "Is Player Too Close?";
}

bool UBTDecorator_IsPlayerTooClose::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	UBlackboardComponent* BB =
		OwnerComp.GetBlackboardComponent();

	if ( !BB ) return false;

	AUK_CharacterBase* Player =
		Cast<AUK_CharacterBase>(BB->GetValueAsObject("Player"));

	if ( !Player ) return false;

	AAIController* AI = OwnerComp.GetAIOwner();
	if ( !AI ) return false;

	APawn* NPC = AI->GetPawn();
	if ( !NPC ) return false;

	float Dist = FVector::Dist(
		NPC->GetActorLocation(),
		Player->GetActorLocation()
	);

	return Dist <= CheckDistance;
}