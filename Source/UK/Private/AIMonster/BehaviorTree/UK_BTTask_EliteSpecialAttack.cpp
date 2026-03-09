#include "AIMonster/BehaviorTree/UK_BTTask_EliteSpecialAttack.h"
#include "AIMonster/Monster/UK_EliteMonster.h"

#pragma region Initialization
UUK_BTTask_EliteSpecialAttack::UUK_BTTask_EliteSpecialAttack()
{
	NodeName = "Elite Special Attack";
}
#pragma endregion

#pragma region Attack Interface
bool UUK_BTTask_EliteSpecialAttack::PlayAttackMontage(AAIMonsterBase* Monster)
{
	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(Monster);
	if (!Elite) return false;
	
	return Elite->PlaySpecialAttack();
}

FSimpleDelegate& UUK_BTTask_EliteSpecialAttack::GetAttackDelegate(AAIMonsterBase* Monster)
{
	static FSimpleDelegate DummyDelegate;
	
	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(Monster);
	if (!Elite) return DummyDelegate;
	
	return Elite->OnSpecialAttackFinished;
}
#pragma endregion