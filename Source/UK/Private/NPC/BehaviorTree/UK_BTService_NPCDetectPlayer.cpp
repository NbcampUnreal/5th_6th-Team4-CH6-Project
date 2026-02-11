#include "NPC/BehaviorTree/UK_BTService_NPCDetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Character/UK_CharacterBase.h"

UUK_BTService_NPCDetectPlayer::UUK_BTService_NPCDetectPlayer()
{
	NodeName = "NPC Detect Player";
	Interval = 0.3f;
}

void UUK_BTService_NPCDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AI = OwnerComp.GetAIOwner();
	if ( !AI ) return;

	APawn* NPC = AI->GetPawn();
	if ( !NPC ) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if ( !BB ) return;

	APlayerController* PC =
		UGameplayStatics::GetPlayerController(NPC->GetWorld(), 0);

	if ( !PC ) return;

	AUK_CharacterBase* Player =
		Cast<AUK_CharacterBase>(
			UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if ( !Player ) return;

	float Dist = FVector::Dist(
		NPC->GetActorLocation(),
		Player->GetActorLocation()
	);

	// 🔹 감지 처리
	if ( Dist <= DetectRadius )
	{
		BB->SetValueAsObject("Player", Player);
		BB->SetValueAsBool("IsAvoiding", true);
	}
	else
	{
		BB->ClearValue("Player");
		BB->SetValueAsBool("IsAvoiding", false);
	}
}