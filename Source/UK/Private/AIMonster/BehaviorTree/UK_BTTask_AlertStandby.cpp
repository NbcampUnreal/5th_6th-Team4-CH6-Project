#include "AIMonster/BehaviorTree/UK_BTTask_AlertStandby.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_AlertStandby::UUK_BTTask_AlertStandby()
{
	NodeName    = "Alert Standby";
	bNotifyTick = true;

	PendingTargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_AlertStandby, PendingTargetKey), AActor::StaticClass());
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_AlertStandby, TargetPlayerKey), AActor::StaticClass());
}

uint16 UUK_BTTask_AlertStandby::GetInstanceMemorySize() const
{
	return sizeof(FAlertStandbyMemory);
}
#pragma endregion

#pragma region Helpers
static void SetOrientToMovement(APawn* Pawn, bool bEnable)
{
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = bEnable;
		}
	}
}

static void SnapFaceToTarget(APawn* Pawn, AActor* Target)
{
	if (!Pawn || !Target) return;

	const FVector  ToTarget = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
	const FRotator FaceRot  = FRotationMatrix::MakeFromX(ToTarget).Rotator();
	Pawn->SetActorRotation(FRotator(0.f, FaceRot.Yaw, 0.f));
}
#pragma endregion

#pragma region Task Interface
EBTNodeResult::Type UUK_BTTask_AlertStandby::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FAlertStandbyMemory* Mem = reinterpret_cast<FAlertStandbyMemory*>(NodeMemory);
	Mem->ElapsedTime = 0.f;

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	AICon->StopMovement();

	SetOrientToMovement(ControlledPawn, true);

	// 즉시 회전하지 않음 - Tick에서 부드럽게 회전

	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		Monster->ShowAlertIcon();

	return EBTNodeResult::InProgress;
}

void UUK_BTTask_AlertStandby::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FAlertStandbyMemory* Mem = reinterpret_cast<FAlertStandbyMemory*>(NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	// PendingTarget 방향으로 시선 부드럽게 보간 
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(PendingTargetKey.SelectedKeyName));
	if (Target)
	{
		const FVector  ToTarget   = (Target->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
		const FRotator TargetRot  = FRotationMatrix::MakeFromX(ToTarget).Rotator();
		const FRotator CurrentRot = ControlledPawn->GetActorRotation();
		const FRotator NewRot     = FMath::RInterpTo(CurrentRot,
		                                              FRotator(0.f, TargetRot.Yaw, 0.f),
		                                              DeltaSeconds,
		                                              LookAtSpeed);
		ControlledPawn->SetActorRotation(NewRot);
	}

	//  AlertDuration 경과 시 추격 전환 
	Mem->ElapsedTime += DeltaSeconds;
	if (Mem->ElapsedTime >= AlertDuration)
	{
		// 느낌표 끄기와 동시에 추격 시작
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
			Monster->HideAlertIcon();

		if (ACharacter* Char = Cast<ACharacter>(ControlledPawn))
		{
			if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement     = true; // 이동 방향 자동회전 OFF
				MC->bUseControllerDesiredRotation = false;  // 컨트롤러 방향으로 회전
			}
		}
		
		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		// PendingTarget → TargetPlayer 복사 → [추격] or [공격] 브랜치 진입
		if (Target)
			BB->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Target);

		BB->ClearValue(PendingTargetKey.SelectedKeyName);

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UUK_BTTask_AlertStandby::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AlertDuration 이내 이탈 → 느낌표 끄기 + 이동 컴포넌트 복원 + PendingTarget 클리어
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (APawn* ControlledPawn = AICon->GetPawn())
		{
			// 기존 SetOrientToMovement(true) 대신 명시적으로 복원
			if (ACharacter* Char = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
				{
					MC->bOrientRotationToMovement     = true; 
					MC->bUseControllerDesiredRotation = false;
				}
				
				AICon->ClearFocus(EAIFocusPriority::Gameplay);
				
			}
			
			if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
				Monster->HideAlertIcon();
		}

		AICon->StopMovement();
	}

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		BB->ClearValue(PendingTargetKey.SelectedKeyName);

	return EBTNodeResult::Aborted;
}
#pragma endregion