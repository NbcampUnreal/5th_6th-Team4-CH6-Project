#include "AIMonster/BehaviorTree/UK_BTTask_EmergeFromGround.h"
#include "AIController.h"
#include "AIMonster/Monster/UK_BurrowMonster.h"
#include "AIMonster/Animation/UKAIMonsterAnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTTask_EmergeFromGround::UUK_BTTask_EmergeFromGround()
{
	NodeName            = "Emerge From Ground";
	bCreateNodeInstance = true;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_EmergeFromGround, TargetPlayerKey), AActor::StaticClass());
}
#pragma endregion

#pragma region Task Interface
EBTNodeResult::Type UUK_BTTask_EmergeFromGround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	Monster->SetBurrowed(false);
    
	if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
		MC->SetMovementMode(MOVE_Walking);

	AICon->StopMovement();

	UAnimInstance* AnimInst = Monster->GetMesh() ? Monster->GetMesh()->GetAnimInstance() : nullptr;

	if (UUKAIMonsterAnimInstance* MonsterAnim = Cast<UUKAIMonsterAnimInstance>(AnimInst))
		MonsterAnim->bShouldEmerge = true;

	UAnimMontage* Montage = Monster->EmergeMontage;
	if (!AnimInst || !Montage)
		return EBTNodeResult::Succeeded;

	const float Length = AnimInst->Montage_Play(Montage, 1.0f);
	if (Length <= 0.f)
		return EBTNodeResult::Succeeded;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UUK_BTTask_EmergeFromGround::OnMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, Montage);

	CachedOwnerComp = &OwnerComp;
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UUK_BTTask_EmergeFromGround::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn()))
		{
			if (UAnimInstance* AnimInst = Monster->GetMesh() ? Monster->GetMesh()->GetAnimInstance() : nullptr)
			{
				if (UUKAIMonsterAnimInstance* MonsterAnim = Cast<UUKAIMonsterAnimInstance>(AnimInst))
					MonsterAnim->bShouldEmerge = false;

				CleanupMontageDelegate(AnimInst, Monster->EmergeMontage);
				AnimInst->StopAllMontages(0.15f);
			}

			// TargetPlayer가 있으면 추격으로 전환 → 지상 상태 유지
			const bool bHasTarget = [&]() -> bool {
				UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
				return BB && BB->GetValueAsObject(TargetPlayerKey.SelectedKeyName) != nullptr;
			}();

			if (!bHasTarget)
				Monster->SetBurrowed(true);
		}
	}

	CachedOwnerComp.Reset();
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Internal
void UUK_BTTask_EmergeFromGround::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!CachedOwnerComp.IsValid()) return;

	UBehaviorTreeComponent& OwnerComp = *CachedOwnerComp.Get();
	CachedOwnerComp.Reset();
	
	FinishLatentTask(OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
}

void UUK_BTTask_EmergeFromGround::CleanupMontageDelegate(UAnimInstance* AnimInst, UAnimMontage* Montage)
{
	if (!AnimInst || !Montage) return;

	FOnMontageEnded EmptyDelegate;
	AnimInst->Montage_SetEndDelegate(EmptyDelegate, Montage);
}
#pragma endregion