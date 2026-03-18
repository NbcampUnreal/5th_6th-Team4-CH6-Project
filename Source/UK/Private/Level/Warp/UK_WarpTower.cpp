#include "Level/Warp/UK_WarpTower.h"
#include "Components/SphereComponent.h"
#include "Character/UK_CharacterBase.h" 
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
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