#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "AIMonster/BossMonster/UK_BossAnimInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

AUK_BossMonsterBase::AUK_BossMonsterBase()
{
	bReplicates = true;
	
	WeaponCollision_R = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision_R"));
	WeaponCollision_R->SetupAttachment(GetMesh());
	WeaponCollision_R->SetCollisionProfileName(TEXT("NoCollision"));
	
	WeaponCollision_L = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision_L"));
	WeaponCollision_L->SetupAttachment(GetMesh());
	WeaponCollision_L->SetCollisionProfileName(TEXT("NoCollision"));
}

void AUK_BossMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = 400.f; 
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	
	WeaponCollision_R->OnComponentBeginOverlap.AddDynamic(this, &AUK_BossMonsterBase::OnWeaponOverlap);
	WeaponCollision_R->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_Right"));
	
	WeaponCollision_L->OnComponentBeginOverlap.AddDynamic(this, &AUK_BossMonsterBase::OnWeaponOverlap);
	WeaponCollision_L->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_Left"));
	
	WeaponCollision_R->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponCollision_L->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	BossAnim = Cast<UUK_BossAnimInstance>(GetMesh()->GetAnimInstance());
	Phase1Tag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Phase1"));
	Phase2Tag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Phase2"));
	Phase3Tag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Phase3"));
	EnrageTag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Enrage"));

	if (HasAuthority())
	{
		SetPhase(Phase1Tag);
	}
}

void AUK_BossMonsterBase::StartAttack()
{
	bIsAttacking = true;
	
	HitActors.Empty();
	SetWeaponCollisionEnabled(true);
    
	UE_LOG(LogTemp, Warning, TEXT("[Boss] Attack Started - Detection ON"));
}

void AUK_BossMonsterBase::EndAttack() 
{
	bIsAttacking = false;
	SetWeaponCollisionEnabled(false);
    
	UE_LOG(LogTemp, Warning, TEXT("[Boss] Attack Ended - Detection OFF"));
}

void AUK_BossMonsterBase::ReceiveDamage(float Damage)
{
	Super::ReceiveDamage(Damage);

	if (HasAuthority())
	{
		UpdatePhase();
	}
}

void AUK_BossMonsterBase::UpdatePhase()
{
	if (!AttributeSet) return;
	const float CurrentHP = AttributeSet->GetHealth();
	const float MaxHP = AttributeSet->GetMaxHealth();
	
	if (MaxHP <= 0.f) return;
	const float HPRatio = CurrentHP / MaxHP;

	if (HPRatio <= EnrageHPRatio)
	{
		SetPhase(EnrageTag);
	}
	else if (HPRatio <= Phase3HPRatio)
	{
		SetPhase(Phase3Tag);
	}
	else if (HPRatio <= Phase2HPRatio)
	{
		SetPhase(Phase2Tag);
	}
}

void AUK_BossMonsterBase::SetPhase(const FGameplayTag& NewPhase)
{
	if (!HasAuthority()) return;

	if (CurrentPhaseTag == NewPhase) return;

	CurrentPhaseTag = NewPhase;

	OnBossPhaseChanged.Broadcast(CurrentPhaseTag);
}

void AUK_BossMonsterBase::OnRep_Phase()
{
	UE_LOG(LogTemp, Log,TEXT("[Boss] Phase Changed : %s"),*CurrentPhaseTag.ToString());
	
	OnBossPhaseChanged.Broadcast(CurrentPhaseTag);
}

bool AUK_BossMonsterBase::PlayRandomAttackMontage()
{
	StartAttack();
	UE_LOG(LogTemp, Warning, TEXT("Boss Try Attack"));
	TArray<UAnimMontage*>* Pattern = nullptr;

	if (CurrentPhaseTag == Phase1Tag)
	{
		Pattern = &Phase1Patterns;
	}
	else if (CurrentPhaseTag == Phase2Tag)
	{
		Pattern = &Phase2Patterns;
	}
	else if (CurrentPhaseTag == Phase3Tag)
	{
		Pattern = &Phase3Patterns;
	}
	else if (CurrentPhaseTag == EnrageTag)
	{
		Pattern = &EnragePatterns;
	}

	if (!Pattern || Pattern->Num() == 0)
		return false;

	const int32 Index =
		FMath::RandRange(0, Pattern->Num() - 1);

	return PlayAnimMontage((*Pattern)[Index]) > 0.f;
}
void AUK_BossMonsterBase::ApplyDamageToTarget(AActor* TargetActor, float SkillDamageMultiplier)
{
	
	if (!TargetActor || !DamageGEClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Target or GE Class is Missing!"));
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent(); 

	if (TargetASC && SourceASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempting to apply damage to: %s"), *TargetActor->GetName());
		
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddInstigator(this, this);

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGEClass, 1.0f, EffectContext);
        
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Damage")), SkillDamageMultiplier);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			UE_LOG(LogTemp, Error, TEXT("Damage Spec Applied Successfully!"));
		}
	}
}

void AUK_BossMonsterBase::SetWeaponCollisionEnabled(bool bEnabled)
{
	ECollisionEnabled::Type NewType = bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;

	if (bEnabled) HitActors.Empty();

	if (WeaponCollision_R)
	{
		WeaponCollision_R->SetCollisionEnabled(NewType);
		
		WeaponCollision_R->SetHiddenInGame(!bEnabled); 
	}

	if (WeaponCollision_L)
	{
		WeaponCollision_L->SetCollisionEnabled(NewType);
		
		WeaponCollision_L->SetHiddenInGame(!bEnabled);
	}
}

void AUK_BossMonsterBase::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
										  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
										  bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || bIsDying) return;
	
	if (bIsAttacking) 
	{
		if (!HitActors.Contains(OtherActor))
		{
			HitActors.Add(OtherActor);
 
			ApplyDamageToTarget(OtherActor, 30.f);

#if !UE_BUILD_SHIPPING
			DrawDebugSphere(GetWorld(), SweepResult.ImpactPoint, 15.f, 8, FColor::Red, false, 1.0f);
#endif
		}
	}
}
void AUK_BossMonsterBase::ShowHPBar()
{
	Super::ShowHPBar();

	if (HPWidgetComponent)
	{
		HPWidgetComponent->SetDrawSize(FVector2D(400.f, 40.f));
	}
}

void AUK_BossMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUK_BossMonsterBase, CurrentPhaseTag);
}
