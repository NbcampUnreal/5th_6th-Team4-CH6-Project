#include "AIMonster/BossMonster/BehaviorTree/UK_BTService_FindUKPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Character/UK_CharacterBase.h"
#include "Engine/World.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"

UUK_BTService_FindUKPlayer::UUK_BTService_FindUKPlayer()
{
	NodeName    = TEXT("Find UK Player");
	bNotifyTick = true;
}

void UUK_BTService_FindUKPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AI  = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AI || !BB) return;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AI->GetPawn());
	if (Boss && (Boss->bIsAttacking || Boss->bIsHit)) return;

	APawn* SelfPawn = AI->GetPawn();
	if (!SelfPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector SelfLoc    = SelfPawn->GetActorLocation();
	const FVector HomeLocation = BB->GetValueAsVector(TEXT("HomeLocation"));

	if (FVector::DistSquared(SelfLoc, HomeLocation) > FMath::Square(5000.f))
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->SetValueAsBool(TEXT("IsSearching"), false);
		return;
	}

	const float DetectRadiusSq = FMath::Square(Boss ? Boss->DetectionRadius : DetectRadius);

	AUK_CharacterBase* ClosestPlayer = nullptr;
	float              BestDistSq    = DetectRadiusSq;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(PC->GetPawn());
		if (!IsValid(Player)) continue;

		// 사망 체크
		if (UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent())
		{
			const UUK_PlayerStatusAttributeSet* PlayerAS = Cast<UUK_PlayerStatusAttributeSet>(
				PlayerASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));
			if (PlayerAS && PlayerAS->GetHealth() <= 0.f) continue;
		}

		const float DistSq = FVector::DistSquared(Player->GetActorLocation(), SelfLoc);
		if (DistSq < BestDistSq)
		{
			BestDistSq    = DistSq;
			ClosestPlayer = Player;
		}
	}

	if (ClosestPlayer)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), ClosestPlayer);
		BB->SetValueAsFloat(TEXT("DistanceToTarget"), FMath::Sqrt(BestDistSq));
		BB->SetValueAsFloat(TEXT("SearchStartTime"), 0.f);
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->ClearValue(TEXT("DistanceToTarget"));

		const float SearchStartTime = BB->GetValueAsFloat(TEXT("SearchStartTime"));

		if (SearchStartTime == -1.f) return; // 이미 수색 실패 확정

		const float Now = World->GetTimeSeconds();

		if (SearchStartTime <= 0.f)
		{
			BB->SetValueAsFloat(TEXT("SearchStartTime"), Now);
		}
		else if (Now - SearchStartTime >= SearchDuration)
		{
			BB->SetValueAsFloat(TEXT("SearchStartTime"), -1.f);
		}
	}
}