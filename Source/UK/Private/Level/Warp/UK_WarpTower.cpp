#include "Level/Warp/UK_WarpTower.h"
#include "Components/SphereComponent.h"
#include "Character/UK_CharacterBase.h" 
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "Level/Warp/UK_WarpSubsystem.h"

AUK_WarpTower::AUK_WarpTower()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(200.f);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMesh->SetupAttachment(RootComponent);
}

void AUK_WarpTower::BeginPlay()
{
	Super::BeginPlay();

	// 델리게이트 바인딩
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_WarpTower::OnOverlapBegin);

	// 게임 시작 시 서브시스템에서 활성화 여부 확인
		if ( UWorld* World = GetWorld() )
		{
			if ( UUK_WarpSubsystem* WarpSubsystem = World->GetSubsystem<UUK_WarpSubsystem>() )
			{
				// 서브시스템에 이미 내 ID가 등록되어 있을 시
				if ( WarpSubsystem->GetActivatedPointIDs().Contains(WarpPointID) )
				{
					bIsActivated = true;

					// 블루프린트에서 만든 시각 효과 로직 실행
					OnTowerActivated();

					UE_LOG(LogTemp, Log, TEXT("WarpTower [%s]: Already Activated in Load Data."), *WarpPointID.ToString());
				}
			}
		}
}

void AUK_WarpTower::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// 캐릭터인지 확인
	AUK_CharacterBase* TargetCharacter = Cast<AUK_CharacterBase>(OtherActor);
	if ( !TargetCharacter ) return;

	// 최초 활성화 로직
	if ( !bIsActivated )
	{
		bIsActivated = true;
		OnTowerActivated();

		if ( UWorld* World = GetWorld() )
		{
			if ( UUK_WarpSubsystem* WarpSubsystem = World->GetSubsystem<UUK_WarpSubsystem>() )
			{
				WarpSubsystem->RegisterWarpPoint(WarpPointID, GetActorLocation());
			}
			
			// 임시 퀘스트 권한 부여
			if ( UGameInstance* GI = GetGameInstance() )
			{
				if ( UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
				{
					QuestSys->EmitQuestEvent(FName(TEXT("QuestEvent.Custom.WarpUnlocked")));
					UE_LOG(LogTemp, Warning, TEXT("[Warp][Quest] Emit WarpUnlocked. Actor=%s"), *GetName());
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Warp Tower Activated & Registered: %s"), *WarpPointID.ToString());
	}

	// 체력/마나 회복 로직 
	if ( UAbilitySystemComponent* ASC = TargetCharacter->GetAbilitySystemComponent() )
	{
		// 최대체력, 마나 가져오기
		const float MaxHP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute());
		const float MaxMP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxMpAttribute());

		// 현재 값을 최대치로 즉시 변경
		ASC->SetNumericAttributeBase(UUK_PlayerStatusAttributeSet::GetHealthAttribute(), MaxHP);
		ASC->SetNumericAttributeBase(UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute(), MaxMP);

		UE_LOG(LogTemp, Log, TEXT("WarpTower: Full Recovery Success (HP: %.f, MP: %.f)"), MaxHP, MaxMP);
	}
}