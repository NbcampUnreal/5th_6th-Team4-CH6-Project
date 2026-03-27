#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_BackPos.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"

UUK_BTDecorator_BackPos::UUK_BTDecorator_BackPos()
{
	NodeName      = TEXT("Back Home Pos");
	bNotifyTick   = true;
	FlowAbortMode = EBTFlowAbortMode::Both;
}

bool UUK_BTDecorator_BackPos::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController*        AI = OwnerComp.GetAIOwner();
	if (!BB || !AI || !AI->GetPawn()) return false;

	AUK_CharacterBase* Target = Cast<AUK_CharacterBase>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (Target)
	{
		if (UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponent())
		{
			const UUK_PlayerStatusAttributeSet* PlayerAS = Cast<UUK_PlayerStatusAttributeSet>(
				ASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));
			if (!PlayerAS) return false;

			return PlayerAS->GetHealth() <= 0.f;
		}
		return false;
	}
	
	const float SearchStartTime = BB->GetValueAsFloat(TEXT("SearchStartTime"));
	if (SearchStartTime > 0.f)  return false;
	if (SearchStartTime < 0.f)  return true; 

	const FVector Home = BB->GetValueAsVector(TEXT("HomeLocation"));
	return FVector::DistSquared(AI->GetPawn()->GetActorLocation(), Home) > FMath::Square(MaxDistance);
}