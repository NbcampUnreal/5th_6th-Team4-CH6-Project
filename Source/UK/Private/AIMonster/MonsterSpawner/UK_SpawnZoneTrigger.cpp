#include "AIMonster/MonsterSpawner/UK_SpawnZoneTrigger.h"
#include "AIMonster/MonsterSpawner/UK_MonsterSpawner.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"

#pragma region Initialization
AUK_SpawnZoneTrigger::AUK_SpawnZoneTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(ActivationRadius);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AUK_SpawnZoneTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->SetSphereRadius(ActivationRadius);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_SpawnZoneTrigger::OnPlayerEnterRange);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_SpawnZoneTrigger::OnPlayerExitRange);

    // 시작 시 풀만 초기화, 스폰은 안 함
    for (AUK_MonsterSpawner* Spawner : ManagedSpawners)
    {
        if (IsValid(Spawner))
        {
            Spawner->InitializeObjectPool();
        }
    }
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		TArray<AActor*> OverlappedActors;
		TriggerSphere->GetOverlappingActors(OverlappedActors);
		
		for (AActor* Actor : OverlappedActors)
		{
			if (Actor && Actor->ActorHasTag(TEXT("SandboxCharacter")))
			{
				PlayerRefCount++;
				if (PlayerRefCount == 1)
				{
					for (AUK_MonsterSpawner* Spawner : ManagedSpawners)
					{
						if (IsValid(Spawner)) Spawner->StartSpawning();
					}
				}
				break;
			}
		}
	});
}
#pragma endregion

#pragma region Overlap
void AUK_SpawnZoneTrigger::OnPlayerEnterRange(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (!OtherActor || !OtherActor->ActorHasTag(TEXT("SandboxCharacter"))) return;

    PlayerRefCount++;
    if (PlayerRefCount != 1) return; // 이미 활성화 중

    for (AUK_MonsterSpawner* Spawner : ManagedSpawners)
    {
        if (IsValid(Spawner))
        {
            Spawner->StartSpawning();
        }
    }
}

void AUK_SpawnZoneTrigger::OnPlayerExitRange(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    if (!OtherActor || !OtherActor->ActorHasTag(TEXT("SandboxCharacter"))) return;

    PlayerRefCount = FMath::Max(0, PlayerRefCount - 1);
    if (PlayerRefCount != 0) return; // 아직 범위 내 플레이어 있음

    for (AUK_MonsterSpawner* Spawner : ManagedSpawners)
    {
        if (!IsValid(Spawner)) continue;

        const bool bActuallyStoped = Spawner->StopSpawning();
    	if (!bActuallyStoped) continue;

        TArray<AAIMonsterBase*> ToDeactivate = Spawner->GetActiveMonsters();
        for (AAIMonsterBase* Monster : ToDeactivate)
        {
            if (!IsValid(Monster) || Monster->IsDead()) continue;
            Spawner->ReturnMonsterToPool(Monster);
        }
    }
}
#pragma endregion