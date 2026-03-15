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
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "DataAsset/DataTable/AIMonster/UK_MonsterCombatRow.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Tags/UK_GameplayTags.h"

AUK_BossMonsterBase::AUK_BossMonsterBase()
{
	bReplicates = true;
	
	WeaponCollision_R = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision_R"));
	WeaponCollision_R->SetupAttachment(GetMesh(),TEXT("weapon_Right"));
	WeaponCollision_R->SetCollisionProfileName(TEXT("NoCollision"));
	
	WeaponCollision_L = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision_L"));
	WeaponCollision_L->SetupAttachment(GetMesh(),TEXT("weapon_Left"));
	WeaponCollision_L->SetCollisionProfileName(TEXT("NoCollision"));
}

void AUK_BossMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = 475.f; 
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	
	WeaponCollision_R->OnComponentBeginOverlap.AddDynamic(this, &AUK_BossMonsterBase::OnWeaponOverlap);
	WeaponCollision_R->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_Right"));
	
	WeaponCollision_L->OnComponentBeginOverlap.AddDynamic(this, &AUK_BossMonsterBase::OnWeaponOverlap);
	WeaponCollision_L->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_Left"));
	
	WeaponCollision_R->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponCollision_L->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void AUK_BossMonsterBase::StartAttack()
{
	bIsAttacking = true;
	
	HitActors.Empty();
	SetWeaponCollisionEnabled(true);

}

void AUK_BossMonsterBase::EndAttack() 
{
	bIsAttacking = false;
	SetWeaponCollisionEnabled(false);
}

void AUK_BossMonsterBase::ReceiveDamage(float Damage)
{
	Super::ReceiveDamage(Damage);
	
	UpdatePhase();
}

void AUK_BossMonsterBase::UpdatePhase()
{
}

void AUK_BossMonsterBase::NotifyAttacked(AController* InstigatorController)
{
	Super::NotifyAttacked(InstigatorController);
	
	if (bIsAttacking || bIsDying) return;
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHitReactTime < 5.0f) return;
	
	if (HitReactMontage)
	{
		PlayMontage(HitReactMontage);
		LastHitReactTime = CurrentTime;
		
		if (AAIController* AICtl = Cast<AAIController>(GetController()))
		{
			AICtl->StopMovement();
		}
	}
}

bool AUK_BossMonsterBase::PlayRandomAttackMontage()
{
	if (bIsHit || bIsAttacking || bIsDying) return false;

	// 페이즈별 패턴 선택하기 임시로 올려둠 이것도
	TArray<UAnimMontage*>& TargetPatterns = (CurrentPhase == 1) ? Phase1Patterns : Phase2Patterns;
    
	if (TargetPatterns.Num() == 0) return Super::PlayRandomAttackMontage();

	const int32 Index = FMath::RandRange(0, TargetPatterns.Num() - 1);
	UAnimMontage* Selected = TargetPatterns[Index];

	if (Selected)
	{
		bIsAttacking = true;
		LastAttackTime = GetWorld()->GetTimeSeconds();
		
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(Selected);
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, Selected);
		}
		return true;
	}
	return false;
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
	
	if(bEnabled)
	{
		FVector Loc = WeaponCollision_R->GetComponentLocation();
		FQuat Rot = WeaponCollision_R->GetComponentQuat();
		float Radius = WeaponCollision_R->GetUnscaledCapsuleRadius();
		float HalfHeight = WeaponCollision_R->GetUnscaledCapsuleHalfHeight();

		DrawDebugCapsule(GetWorld(), Loc, HalfHeight, Radius, Rot, FColor::Blue, false, 0.5f);
	}
}

void AUK_BossMonsterBase::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
										  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
										  bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsAttacking || HitActors.Contains(OtherActor)) return;

	if (AUK_CharacterBase* TargetPlayer = Cast<AUK_CharacterBase>(OtherActor))
	{
		UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();
        
		if (TargetASC)
		{
			HitActors.Add(OtherActor); 
			const UUK_PlayerStatusAttributeSet* PlayerStats = Cast<UUK_PlayerStatusAttributeSet>(TargetASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass()));

			if (PlayerStats)
			{
				float BeforeHP = PlayerStats->GetHealth();

				TargetASC->ApplyModToAttribute(
					UUK_PlayerStatusAttributeSet::GetDamageAttribute(), 
					EGameplayModOp::Additive, 
					AttackDamage
				);

				FVector HitLocation = OtherActor->GetActorLocation();
				DrawDebugSphere(GetWorld(), HitLocation, 50.f, 12, FColor::Red, false, 2.0f);
				DrawDebugString(GetWorld(), HitLocation + FVector(0.f, 0.f, 100.f), TEXT("!!! HIT !!!"), nullptr, FColor::Yellow, 1.5f);
			
				if (MonsterCombatTable)
				{
					FString EnumStr = UEnum::GetValueAsString(MonsterType);
					FString RowStr;
					EnumStr.Split(TEXT("::"), nullptr, &RowStr);

					if (FUK_MonsterCombatRow* Row = MonsterCombatTable->FindRow<FUK_MonsterCombatRow>(FName(*RowStr), TEXT("")))
					{
						EHitReactionType HitType = (CurrentAttackType == EMonsterAttackType::Normal)
							? Row->NormalAttackHit
							: Row->SpecialAttackHit;

						FGameplayEventData EventData;
						EventData.EventMagnitude = (float)HitType;
						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
							TargetPlayer,
							UK_GameplayTags::Action::BeAttacked,
							EventData);
					}
				}
			}
		}
	}
}

AActor* AUK_BossMonsterBase::GetTargetActor() const
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC && AIC->GetBlackboardComponent())
	{
		return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	}
	return nullptr;
}

float AUK_BossMonsterBase::PlayMontage(UAnimMontage* Montage, float InPlayRate)
{
	if (Montage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			return AnimInstance->Montage_Play(Montage, InPlayRate);
		}
	}
	return 0.f;
}
