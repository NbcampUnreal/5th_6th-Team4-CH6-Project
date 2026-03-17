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

void UUK_BTService_FindUKPlayer::TickNode(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	auto* AI = OwnerComp.GetAIOwner();
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!AI || !BB) return;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AI->GetPawn());
	if (Boss && Boss->bIsAttacking|| Boss->bIsHit) 
	{
		return; 
	}
	
	APawn* SelfPawn = AI->GetPawn();
	if (!SelfPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;
	FVector HomeLocation = BB->GetValueAsVector(TEXT("HomeLocation"));

	float MaxDistance = 3500.f;

	float DistFromHome = FVector::Dist(SelfPawn->GetActorLocation(),HomeLocation);

	if (DistFromHome > MaxDistance)
	{
		BB->ClearValue(TEXT("TargetActor"));
		return;
	}

	AUK_CharacterBase* ClosestPlayer = nullptr;
	float MinDist = DetectRadius;

	for (TActorIterator<AUK_CharacterBase> It(World); It; ++It)
	{
		AUK_CharacterBase* Player = *It;

		if (!IsValid(Player)) continue;

		if (UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent())
		{
			const UUK_PlayerStatusAttributeSet* PlayerAS = Cast<UUK_PlayerStatusAttributeSet>(PlayerASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));
           
			if (PlayerAS && PlayerAS->GetHealth() <= 0.f)
			{
				continue;
			}
		}
		
		float Dist = FVector::Dist(Player->GetActorLocation(),SelfPawn->GetActorLocation());
		if (Dist <= MinDist)
		{
			MinDist = Dist;
			ClosestPlayer = Player;
		}
	}

	if (ClosestPlayer)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), ClosestPlayer);
		float Distance = FVector::Dist(SelfPawn->GetActorLocation(), ClosestPlayer->GetActorLocation());
		BB->SetValueAsFloat(TEXT("DistanceToTarget"), Distance);
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->ClearValue(TEXT("DistanceToTarget"));
		
		if (Boss)
		{
			AUK_BossMonster_Grux* Grux = Cast<AUK_BossMonster_Grux>(Boss);
			if (Grux)
			{
				Grux->CurrentPhase = 1;
				Grux->UpdatePhase(); 
			}
			
			auto* MonsterAS = Boss->GetMonsterAttributeSet();
           
			if (MonsterAS)
			{
				float MaxHP = MonsterAS->GetMaxHealth();
				MonsterAS->SetHealth(MaxHP);
			}
		}
	}
	
}