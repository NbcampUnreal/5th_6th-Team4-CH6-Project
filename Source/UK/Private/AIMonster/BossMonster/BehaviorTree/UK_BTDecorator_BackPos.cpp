#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_BackPos.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

UUK_BTDecorator_BackPos::UUK_BTDecorator_BackPos()
{
	NodeName = TEXT("Back Home Pos");
	bNotifyTick = true;
	FlowAbortMode = EBTFlowAbortMode::Both;
}

bool UUK_BTDecorator_BackPos::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	auto* AI = OwnerComp.GetAIOwner();
	if (!BB || !AI || !AI->GetPawn()) return false;

	AUK_CharacterBase* Target = Cast<AUK_CharacterBase>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (Target)
	{
		if (UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponent())
		{
			auto* PlayerAS = Cast<UUK_PlayerStatusAttributeSet>(
				ASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));
			if (PlayerAS && PlayerAS->GetHealth() > 0.f)
				return false; // 살아있으면 귀환 차단
			if (PlayerAS && PlayerAS->GetHealth() <= 0.f)
				return true;  // 사망 시 귀환
		}
		return false;
	}

	// 타겟 없음 → SearchStartTime 체크
	float SearchStartTime = BB->GetValueAsFloat(TEXT("SearchStartTime"));
	if (SearchStartTime > 0.f) return false; // 수색 중 귀환 차단
	if (SearchStartTime < 0.f) return true;  // 수색 실패 귀환 허용

	// 홈 거리 체크
	FVector Home = BB->GetValueAsVector(TEXT("HomeLocation"));
	float Dist = FVector::Dist(AI->GetPawn()->GetActorLocation(), Home);
	return Dist > MaxDistance;
}