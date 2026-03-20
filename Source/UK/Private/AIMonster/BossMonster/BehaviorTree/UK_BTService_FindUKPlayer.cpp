#include "AIMonster/BossMonster/BehaviorTree/UK_BTService_FindUKPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Character/UK_CharacterBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"

UUK_BTService_FindUKPlayer::UUK_BTService_FindUKPlayer()
{
	NodeName = TEXT("Find UK Player");
	bNotifyTick = true;
}

void UUK_BTService_FindUKPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    auto* AI = OwnerComp.GetAIOwner();
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!AI || !BB) return;

    AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AI->GetPawn());
    if (Boss && (Boss->bIsAttacking || Boss->bIsHit)) return;

    APawn* SelfPawn = AI->GetPawn();
    if (!SelfPawn) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector HomeLocation = BB->GetValueAsVector(TEXT("HomeLocation"));
    float DistFromHome = FVector::Dist(SelfPawn->GetActorLocation(), HomeLocation);

    if (DistFromHome > 5000.f)
    {
        BB->ClearValue(TEXT("TargetActor"));
        BB->SetValueAsBool(TEXT("IsSearching"), false);
        return;
    }

    AUK_CharacterBase* ClosestPlayer = nullptr;
    float MinDist = Boss ? Boss->DetectionRadius : DetectRadius;

    for (TActorIterator<AUK_CharacterBase> It(World); It; ++It)
    {
        AUK_CharacterBase* Player = *It;
        if (!IsValid(Player)) continue;

        if (UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent())
        {
            const UUK_PlayerStatusAttributeSet* PlayerAS = Cast<UUK_PlayerStatusAttributeSet>(
                PlayerASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));
            if (PlayerAS && PlayerAS->GetHealth() <= 0.f) continue;
        }

        float Dist = FVector::Dist(Player->GetActorLocation(), SelfPawn->GetActorLocation());
        if (Dist <= MinDist)
        {
            MinDist = Dist;
            ClosestPlayer = Player;
        }
    }

	if (ClosestPlayer)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), ClosestPlayer);
		BB->SetValueAsFloat(TEXT("DistanceToTarget"),
			FVector::Dist(SelfPawn->GetActorLocation(), ClosestPlayer->GetActorLocation()));
		BB->SetValueAsFloat(TEXT("SearchStartTime"), 0.f);
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->ClearValue(TEXT("DistanceToTarget"));

		float SearchStartTime = BB->GetValueAsFloat(TEXT("SearchStartTime"));
		
		if (SearchStartTime == -1.f)
		{
			return;
		}
		
		if (SearchStartTime <= 0.f)
		{
			// 수색 시작 — 현재 시간 기록
			BB->SetValueAsFloat(TEXT("SearchStartTime"), GetWorld()->GetTimeSeconds());
			UE_LOG(LogTemp, Warning, TEXT("[Boss] 타겟 소실 - 30초 수색 시작"));
		}
		else
		{
			float Elapsed = GetWorld()->GetTimeSeconds() - SearchStartTime;
			if (Elapsed >= SearchDuration)
			{
				BB->SetValueAsFloat(TEXT("SearchStartTime"), -1.f);
				UE_LOG(LogTemp, Warning, TEXT("[Boss] 수색 실패(%.1f초) - 귀환"), Elapsed);
			}
		}
	}
}