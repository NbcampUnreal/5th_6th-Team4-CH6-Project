#include "AIMonster/BossMonster/UK_BossProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/UK_CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "NiagaraFunctionLibrary.h"

AUK_BossProjectileBase::AUK_BossProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	Collisioncomp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	Collisioncomp->InitSphereRadius(50.0f);

	Collisioncomp->SetCollisionProfileName(TEXT("Trigger")); 
	Collisioncomp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
	RootComponent = Collisioncomp;

	ProjectileMovementcomp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovementcomp->UpdatedComponent = Collisioncomp;
	ProjectileMovementcomp->InitialSpeed = 600.f;
	ProjectileMovementcomp->MaxSpeed = 2000.f;
	ProjectileMovementcomp->bRotationFollowsVelocity = true;
	ProjectileMovementcomp->ProjectileGravityScale = 0.f;

	Collisioncomp->OnComponentBeginOverlap.AddDynamic(this, &AUK_BossProjectileBase::OnOverlapBegin);
	
}

void AUK_BossProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(4.0f);
}

void AUK_BossProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner()) return;
	
	if (AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor))
	{
		if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
		{
			ASC->ApplyModToAttribute(UUK_PlayerStatusAttributeSet::GetDamageAttribute(),EGameplayModOp::Additive,DamageAmount);
			if (HitEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation());
			}
			Destroy();
		}
	}
}

