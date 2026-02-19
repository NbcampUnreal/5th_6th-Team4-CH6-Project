#include "AIMonster/BehaviorTree/UK_BTTask_Attack.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UUK_BTTask_Attack::UUK_BTTask_Attack()
{
	NodeName = "Attack (Montage)";
	bNotifyTick = true;
	bCreateNodeInstance = true;  // 인스턴스별 상태 보관
}

EBTNodeResult::Type UUK_BTTask_Attack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || Monster->IsDead()) return EBTNodeResult::Failed;

	// 초기화
	bMontageStarted = false;
	FacingWaitElapsed = 0.f;

	//TickTask에서 회전 체크 후 공격
	return EBTNodeResult::InProgress;
}

void UUK_BTTask_Attack::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || Monster->IsDead())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!bMontageStarted)
	{
		FacingWaitElapsed += DeltaSeconds;

		// 타겟 방향 체크
		bool bFacingTarget = false;

		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		if (BB)
		{
			AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer")));
			if (Target)
			{
				FVector ToTarget = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal2D();
				FVector MonsterForward = Monster->GetActorForwardVector().GetSafeNormal2D();

				float DotProduct = FVector::DotProduct(MonsterForward, ToTarget);
				float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f)));

				bFacingTarget = (AngleDeg <= FacingAngleTolerance);
			}
			else
			{
				// 타겟 없으면 그냥 공격
				bFacingTarget = true;
			}
		}
		else
		{
			bFacingTarget = true;
		}

		// 타겟을 바라보고 있거나, 대기 시간 초과 시 공격 시작
		if (bFacingTarget || FacingWaitElapsed >= MaxFacingWaitTime)
		{
			if (!Monster->PlayRandomAttackMontage())
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}

			bMontageStarted = true;
		}

		return;  // 아직 회전 중이면 다음 틱 대기
	}

	if (!Monster->bIsAttacking)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UUK_BTTask_Attack::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
		if (Monster)
		{
			Monster->bIsAttacking = false;
			if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
			{
				Anim->StopAllMontages(0.25f);
			}
		}
	}

	bMontageStarted = false;
	FacingWaitElapsed = 0.f;
	return EBTNodeResult::Aborted;
}