#include "AIMonster/BehaviorTree/UK_BTService_SetWalkSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTService_SetWalkSpeed::UUK_BTService_SetWalkSpeed()
{
	NodeName = "Set Walk Speed";
	Interval = 0.f;
	RandomDeviation = 0.f;
}
#pragma endregion

#pragma region Speed Management
void UUK_BTService_SetWalkSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return;

	CachedOriginalSpeed    = MoveComp->MaxWalkSpeed;
	MoveComp->MaxWalkSpeed = WanderSpeed;
}

void UUK_BTService_SetWalkSpeed::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return;

	MoveComp->MaxWalkSpeed = (OriginalSpeed > 0.f) ? OriginalSpeed : CachedOriginalSpeed;
}
#pragma endregion