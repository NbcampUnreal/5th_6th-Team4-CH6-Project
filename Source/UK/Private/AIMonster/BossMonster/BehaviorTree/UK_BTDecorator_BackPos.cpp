#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_BackPos.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"

UUK_BTDecorator_BackPos::UUK_BTDecorator_BackPos()
{
	NodeName = TEXT("Back Home Pos");
	bNotifyTick = true;
	FlowAbortMode = EBTFlowAbortMode::Both;
}

bool UUK_BTDecorator_BackPos::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	auto* AI = OwnerComp.GetAIOwner();
	if (!BB || !AI || !AI->GetPawn()) return false;

	APawn* Pawn = AI->GetPawn();
	AUK_CharacterBase* Target = Cast<AUK_CharacterBase>(BB->GetValueAsObject(TEXT("TargetActor")));
	bool bIsPlayerDead = false;

	if (Target)
	{
		if (UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponent())
		{
			auto* PlayerAS = Cast<UUK_PlayerStatusAttributeSet>(ASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));
			if (PlayerAS && PlayerAS->GetHealth() <= 0.f)
			{
				bIsPlayerDead = true;
			}
		}
	}
	
	FVector Home = BB->GetValueAsVector(TEXT("HomeLocation"));
	float Dist = FVector::Dist(Pawn->GetActorLocation(), Home);
	
	return bIsPlayerDead || (Dist > MaxDistance);
}