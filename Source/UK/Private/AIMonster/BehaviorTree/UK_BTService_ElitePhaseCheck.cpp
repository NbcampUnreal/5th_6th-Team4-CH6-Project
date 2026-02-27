#include "AIMonster/BehaviorTree/UK_BTService_ElitePhaseCheck.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/Monster/UK_EliteMonster.h"

#pragma region Initialization
UUK_BTService_ElitePhaseCheck::UUK_BTService_ElitePhaseCheck()
{
	NodeName        = "Elite Special Attack Check";
	Interval        = 0.2f;
	RandomDeviation = 0.05f;

	CanSpecialAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_ElitePhaseCheck, CanSpecialAttackKey));
}
#pragma endregion

#pragma region Phase Check
void UUK_BTService_ElitePhaseCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn());
	if (!Elite) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	BB->SetValueAsBool(CanSpecialAttackKey.SelectedKeyName, Elite->CanUseSpecialAttack());
}
#pragma endregion