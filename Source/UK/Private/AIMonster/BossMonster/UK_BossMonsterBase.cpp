#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AUK_BossMonsterBase::AUK_BossMonsterBase()
{
	bReplicates = true;
}

void AUK_BossMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = 200.f; 
	
	Phase1Tag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Phase1"));
	Phase2Tag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Phase2"));
	Phase3Tag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Phase3"));
	EnrageTag  = FGameplayTag::RequestGameplayTag(TEXT("Boss.Phase.Enrage"));

	if (HasAuthority())
	{
		SetPhase(Phase1Tag);
	}
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
