#include "AIMonster/BehaviorTree/UK_BTTask_Attack.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_Attack::UUK_BTTask_Attack()
{
	NodeName = "Attack (Montage)";
}
#pragma endregion

#pragma region Attack Interface
bool UUK_BTTask_Attack::PlayAttackMontage(AAIMonsterBase* Monster)
{
	if (!Monster) return false;
	return Monster->PlayRandomAttackMontage();
}

FSimpleDelegate& UUK_BTTask_Attack::GetAttackDelegate(AAIMonsterBase* Monster)
{
	static FSimpleDelegate DummyDelegate;
	if (!Monster) return DummyDelegate;
	
	return Monster->OnAttackFinished;
}
#pragma endregion