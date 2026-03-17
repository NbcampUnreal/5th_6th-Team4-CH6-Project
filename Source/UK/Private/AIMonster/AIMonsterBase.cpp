#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "Tags/UK_GameplayTags.h"
#include "DataAsset/DataTable/AIMonster/UK_MonsterLootRow.h"
#include "ActorComponent/UK_InventoryComponent.h"

#pragma region Initialization
AAIMonsterBase::AAIMonsterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false); 

	// Attribute Set
	AttributeSet = CreateDefaultSubobject<UUK_MonsterAttributeSet>(TEXT("AttributeSet"));

	bUseControllerRotationYaw = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement     = true;   
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate                  = FRotator(0.f, 240.f, 0.f);  
	}

	// HP Widget 설치
	HPWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidgetComponent"));
	HPWidgetComponent->SetupAttachment(GetMesh());
	HPWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPWidgetComponent->SetDrawAtDesiredSize(true);
	HPWidgetComponent->SetDrawSize(FVector2D(180.f, 20.f));
	HPWidgetComponent->SetRelativeLocation(FVector(0, 0, 120.f));
	HPWidgetComponent->SetVisibility(false);
	
	// Alert Icon Widget 설치 (HPBar 바로 위)
	AlertWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AlertWidgetComponent"));
	AlertWidgetComponent->SetupAttachment(RootComponent);
	AlertWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	AlertWidgetComponent->SetDrawAtDesiredSize(true);  
	AlertWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	AlertWidgetComponent->SetVisibility(false);
	AlertWidgetComponent->SetCullDistance(AlertWidgetCullDistance);
	AlertWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 225.f));
}

void AAIMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = false;
	}

	SpawnLocation = GetActorLocation();

	// AbilitySystemComponent 초기화
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->InitStats(UUK_MonsterAttributeSet::StaticClass(), nullptr);
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsVector(TEXT("SpawnLocation"), SpawnLocation);
		}
	}

	if (Personality == EMonsterPersonality::Peaceful)
	{
		CurrentState = EMonsterState::Passive;
	}

	if (HPWidgetComponent && GetWorld())
	{
		GetWorldTimerManager().SetTimer(
			HPBarUpdateTimer, this,
			&AAIMonsterBase::UpdateHPBarWidget,
			0.05f, true);
	}

	if (HPWidgetComponent)
	{
		HPWidget = Cast<UUK_MonsterHealthBar>(HPWidgetComponent->GetUserWidgetObject());

		if (HPWidget)
		{
			HPWidget->BindMonsterAttributes(
				AbilitySystemComponent,
				AttributeSet
			);
		}
	}
	
	if (AlertWidgetComponent && AlertWidget)
	{
		AlertWidget->SetVisibility(ESlateVisibility::Collapsed);
		AlertWidgetComponent->SetVisibility(false);
		AlertWidgetComponent->SetHiddenInGame(true);
		
		UE_LOG(LogTemp, Log, TEXT("[Alert] %s: Widget hidden in BeginPlay ✓"), *GetName());
	}

	// ── 플레이어 레벨 기반 스탯 자동 초기화 ─────────────────────────────
	AutoInitStatsFromNearestPlayer();
}


void AAIMonsterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AAIMonsterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HPWidgetComponent && HPWidgetClass)
	{
		HPWidgetComponent->SetWidgetClass(HPWidgetClass);
	}
	
	if (AlertWidgetComponent && AlertWidgetClass)
	{
		AlertWidgetComponent->SetWidgetClass(AlertWidgetClass);
		AlertWidgetComponent->InitWidget();
		
		AlertWidget = AlertWidgetComponent->GetUserWidgetObject();
		
		if (AlertWidget)
		{
			AlertWidget->SetVisibility(ESlateVisibility::Collapsed);
			UE_LOG(LogTemp, Log, TEXT("[Alert] %s: Widget initialized ✓"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Alert] %s: Widget initialization failed ✗"), *GetName());
		}
	}
	else
	{
		if (!AlertWidgetClass)
			UE_LOG(LogTemp, Warning, TEXT("[Alert] %s: AlertWidgetClass not set in BP"), *GetName());
	}
}

UAbilitySystemComponent* AAIMonsterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 현재 복제가 필요한 UPROPERTY(Replicated) 변수가 없으므로 부모 호출만 유지
	// 추후 복제 변수 추가 시 여기에 DOREPLIFETIME 매크로 추가
}

void AAIMonsterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UK_GameplayTags::Action::Parry)
			.AddUObject(this, &AAIMonsterBase::OnParryGameplayEvent);
	}

}
#pragma endregion

#pragma region State Management
void AAIMonsterBase::RequestState(EMonsterState NewState)
{
	if (CurrentState == NewState) return;
	SetState(NewState);
}

void AAIMonsterBase::SetState(EMonsterState NewState)
{
	EMonsterState OldState = CurrentState;
	CurrentState = NewState;

	// 상태별 회전 업데이트 자동 제어
	switch (NewState)
	{
	case EMonsterState::Chase:
	case EMonsterState::Attack:
	case EMonsterState::Alert:
	case EMonsterState::Aggressive:
		StartRotationUpdate();
		break;
            
	case EMonsterState::Idle:
	case EMonsterState::Patrol:
	case EMonsterState::Dead:
	case EMonsterState::Passive:
		StopRotationUpdate();
		break;
	}

	OnStateChanged.Broadcast(OldState, NewState);
}
#pragma endregion

#pragma region State Callbacks
void AAIMonsterBase::SetAIActive(bool bActive) {}
void AAIMonsterBase::OnIdle()                  {}
void AAIMonsterBase::OnPatrol()                {}

void AAIMonsterBase::OnChase(float DeltaSeconds) 
{
	if (bUseSmoothRotation)
	{
		StartRotationUpdate();
	}
}

void AAIMonsterBase::OnDead()
{
	StopRotationUpdate();
}

void AAIMonsterBase::OnPassive()
{
	StopRotationUpdate();
}

void AAIMonsterBase::OnAlert()
{
	if (bUseSmoothRotation)
	{
		StartRotationUpdate();
	}
}

void AAIMonsterBase::OnAttack()
{
	PlayRandomAttackMontage();
}
#pragma endregion

#pragma region Combat
void AAIMonsterBase::ApplyDamage(float DamageAmount, AController* InstigatorController)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyDamage Called: %.1f"), *GetName(), DamageAmount);
	
	if (bIsDying)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ✗ Already Dying - Ignoring Damage"), *GetName());
		return;
	}
	
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ✗✗✗ CRITICAL: No AbilitySystemComponent!"), *GetName());
		return;
	}
	
	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ✗✗✗ CRITICAL: DamageEffectClass NOT SET!"), *GetName());
		UE_LOG(LogTemp, Error, TEXT("   → 블루프린트에서 DamageEffectClass를 설정해야 합니다!"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[%s] ✓ ASC Valid, DamageEffectClass Valid"), *GetName());

	// 공격자 정보 없으면 가장 가까운 플레이어 자동 탐지
	if (!InstigatorController)
	{
		float ClosestDistance = 1000.0f;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;

			const float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				InstigatorController = PC;
				ClosestDistance = Distance;
			}
		}
	}

	// 공격자 저장
	if (InstigatorController && InstigatorController->IsA(APlayerController::StaticClass()))
	{
		LastAttackerController = Cast<APlayerController>(InstigatorController);
		UE_LOG(LogTemp, Log, TEXT("[%s] Attacker: %s"), *GetName(), 
			*LastAttackerController->GetName());
	}

	// GameplayEffect로 데미지 적용
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	AActor* InstigatorPawn = (InstigatorController && InstigatorController->GetPawn())
	? InstigatorController->GetPawn()
	: nullptr;
	EffectContext.AddInstigator(InstigatorPawn, this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectClass, 1.0f, EffectContext);

	if (SpecHandle.IsValid())
	{
		// 네이티브 태그 사용 (UK_GameplayTags::Data::Damage)
		SpecHandle.Data->SetSetByCallerMagnitude(UK_GameplayTags::Data::Damage, DamageAmount);
		
		UE_LOG(LogTemp, Warning, TEXT("[%s] ✓ Applying GameplayEffect: %.1f damage"), 
			*GetName(), DamageAmount);
		UE_LOG(LogTemp, Log, TEXT("   Current Health: %.1f / %.1f"), 
			AttributeSet ? AttributeSet->GetHealth() : -1.0f,
			AttributeSet ? AttributeSet->GetMaxHealth() : -1.0f);
		
		FActiveGameplayEffectHandle ActiveHandle = 
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		
		if (ActiveHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] ✓ GameplayEffect Applied Successfully"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] ✗ GameplayEffect Application Failed!"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ✗✗✗ CRITICAL: Invalid SpecHandle!"), *GetName());
		UE_LOG(LogTemp, Error, TEXT("   → DamageEffectClass가 올바르게 설정되었는지 확인"));
	}
	
	// 히트 애니메이션
	if (!bIsDying)
	{
		PlayRandomHitMontage();
	}

	// 평화 몬스터: 피격 시 적대 전환
	if (Personality == EMonsterPersonality::Peaceful && !bIsAggressive)
	{
		AActor* Attacker = nullptr;
		if (InstigatorController && InstigatorController->GetPawn())
		{
			Attacker = InstigatorController->GetPawn();
		}
		else
		{
			// InstigatorController 없을 때만 근처 탐색 (DetectionRadius 활용)
			float ClosestDist = DetectionRadius;
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				if (!PC || !PC->GetPawn()) continue;
				const float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
				if (Dist < ClosestDist)
				{
					ClosestDist = Dist;
					Attacker = PC->GetPawn();
				}
			}
		}

		if (Attacker)
		{
			bIsAggressive = true;
			Aggressor      = Attacker;

			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					BB->SetValueAsObject(TEXT("TargetPlayer"), Attacker);
				}
			}

			CallNearbyAllies(Attacker);
			OnAttacked.Broadcast(this, Attacker);
			RequestState(EMonsterState::Aggressive);

			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
				{
					BTComp->RestartTree();
				}
			}
		}
	}
}

void AAIMonsterBase::ReceiveDamage(float Damage)
{
	ApplyDamage(Damage, nullptr);
}

void AAIMonsterBase::ReceiveDamageFrom(float Damage, AController* InstigatorController)
{
	ApplyDamage(Damage, InstigatorController);
}

void AAIMonsterBase::NotifyAttacked(AController* InstigatorController)
{
	if (Personality != EMonsterPersonality::Peaceful || bIsAggressive) return;

	AActor* Attacker = nullptr;
	if (InstigatorController && InstigatorController->GetPawn())
	{
		Attacker = InstigatorController->GetPawn();
	}
	else
	{
		float ClosestDist = DetectionRadius;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;
			const float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Dist < ClosestDist) { ClosestDist = Dist; Attacker = PC->GetPawn(); }
		}
	}

	if (!Attacker) return;

	bIsAggressive = true;
	Aggressor = Attacker;

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsObject(TEXT("TargetPlayer"), Attacker);  // TargetPlayer 직접 설정
		}
		if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
		{
			BTComp->RestartTree();
		}
	}

	CallNearbyAllies(Attacker);
	OnAttacked.Broadcast(this, Attacker);
	RequestState(EMonsterState::Aggressive);
}

void AAIMonsterBase::OnParryGameplayEvent(const FGameplayEventData* Payload)
{
	HandleParryReaction();
}

void AAIMonsterBase::HandleParryReaction()
{
	if (bIsDying) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.2f); 
	}

	bIsAttacking = false;
	bIsHit = true;

	if (StaggerMontage)
	{
		if (AAIController* AICtl = Cast<AAIController>(GetController()))
		{
			if (UBrainComponent* Brain = AICtl->GetBrainComponent())
			{
				Brain->PauseLogic(TEXT("Parried"));
			}
		}
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();   
		}
		
		float Duration = PlayAnimMontage(StaggerMontage);

		// 몽타주가 끝나면 실행될 콜백 등록해두는 용도
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAIMonsterBase::OnStaggerMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, StaggerMontage);
	}
}
void AAIMonsterBase::OnStaggerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsHit = false; 
	if (AAIController* AICtl = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AICtl->GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("StunEnd"));
		}
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetDefaultMovementMode();
	}
}
#pragma endregion

#pragma region Attack Animation
bool AAIMonsterBase::PlayRandomAttackMontage()
{
	if (bIsHit || bIsAttacking || bIsDying) return false;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return false;

	if (AttackMontages.Num() == 0) return false;

	const int32 RandomIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
	if (!AttackMontages[RandomIndex]) return false;

	bIsAttacking   = true;
	LastAttackTime = Now;

	PlayAttackMontage(RandomIndex);
	return true;
}

void AAIMonsterBase::PlayAttackMontage(int32 MontageIndex)
{
	if (bIsHit) return;
	if (!AttackMontages.IsValidIndex(MontageIndex)) return;
	if (!AttackMontages[MontageIndex]) return;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AAIMonsterBase::OnAttackMontageEnded);
	AnimInstance->Montage_Play(AttackMontages[MontageIndex]);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AAIMonsterBase::OnAttackMontageEnded);
}

void AAIMonsterBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AAIMonsterBase::OnAttackMontageEnded);
	}
	
	bIsAttacking = false;
	OnAttackFinished.ExecuteIfBound();
}
#pragma endregion

#pragma region Idle Animation
bool AAIMonsterBase::PlayRandomIdleMontage()
{
	if (IdleMontages.Num() == 0) return false;

	TArray<int32> ValidIndices;
	for (int32 i = 0; i < IdleMontages.Num(); ++i)
	{
		if (IdleMontages[i]) ValidIndices.Add(i);
	}
	if (ValidIndices.Num() == 0) return false;

	const int32 PickedIndex = ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)];
	PlayIdleMontage(PickedIndex);
	return true;
}

void AAIMonsterBase::PlayIdleMontage(int32 MontageIndex)
{
	if (!IdleMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = IdleMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(Montage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AAIMonsterBase::OnIdleMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
}

void AAIMonsterBase::OnIdleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnIdleMontageFinished.ExecuteIfBound();
}
#pragma endregion

#pragma region Hit Animation
bool AAIMonsterBase::PlayRandomHitMontage()
{
	if (bIsDying) return false;
	if (HitMontages.Num() == 0) return false;

	TArray<int32> ValidIndices;
	for (int32 i = 0; i < HitMontages.Num(); ++i)
	{
		if (HitMontages[i]) ValidIndices.Add(i);
	}
	if (ValidIndices.Num() == 0) return false;

	const int32 PickedIndex = ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)];
	PlayHitMontage(PickedIndex);
	return true;
}

void AAIMonsterBase::PlayHitMontage(int32 MontageIndex)
{
	if (!HitMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = HitMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	AnimInstance->StopAllMontages(0.1f);
	AnimInstance->Montage_Play(Montage);

	bIsHit = true;
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AAIMonsterBase::OnHitMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AAIMonsterBase::OnHitMontageEnded);
}

void AAIMonsterBase::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (HitMontages.Contains(Montage))
	{
		bIsHit = false;
	}
}
#pragma endregion

#pragma region Death System
void AAIMonsterBase::Die()
{
	if (bIsDying) return;

	bIsDying     = true;
	bIsAttacking = false;
	
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// AI 정지
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.1f); 
	}

	SetState(EMonsterState::Dead);
	
	if (const FUK_MonsterMetaRow* Meta = GetMetaRow())
	{
		if (!Meta->MobEntityId.IsNone())
		{
			if (UUKQuestManagerSubsystem* QM =
				GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>())
			{
				const FName EventID(*FString::Printf(
					TEXT("QuestEvent.Killed.%s"), *Meta->MobEntityId.ToString()));
				QM->EmitQuestEvent(EventID);
			}
		}
	}

	GrantRewardsToKiller();

	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(
		DeathTimer, this,
		&AAIMonsterBase::FinalizeDeath,
		CorpseLingerTime > 0.f ? 1.0f : 0.1f,
		false);
}

void AAIMonsterBase::FinalizeDeath()
{
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	if (CorpseLingerTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			CorpseTimerHandle, this,
			&AAIMonsterBase::HideAndBroadcastDeath,
			CorpseLingerTime, false);
	}
	else
	{
		HideAndBroadcastDeath();
	}
}

void AAIMonsterBase::HideAndBroadcastDeath()
{
	HideCorpse();

	OnMonsterKilled.Broadcast(this, MonsterType, LastAttackerController);
	OnDeath.Broadcast(this);
}


void AAIMonsterBase::HideCorpse()
{
	SetActorHiddenInGame(true);
}
#pragma endregion

#pragma region Respawn
void AAIMonsterBase::ResetHealth()
{
	if (!AbilitySystemComponent || !AttributeSet) return;
	
	const float MaxHP = AttributeSet->GetMaxHealth();
	AbilitySystemComponent->SetNumericAttributeBase(
		AttributeSet->GetHealthAttribute(), MaxHP
	);
	
	bIsDying = false;
	bRewardGranted = false;
	
	UE_LOG(LogTemp, Log, TEXT("[%s] ResetHealth: HP restored to %.0f"), *GetName(), MaxHP);
}

void AAIMonsterBase::ResetAppearance()
{
	SetActorHiddenInGame(false);

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetAllBodiesSimulatePhysics(false);
	}

	UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.f);
		AnimInstance->StopAllMontages(0.f);

		if (UAnimMontage* Current = AnimInstance->GetCurrentActiveMontage())
		{
			AnimInstance->Montage_Stop(0.f, Current);
		}
	}

	// AnimInstance 완전 재초기화
	if (MeshComp && MeshComp->AnimClass)
	{
		UClass* AnimClass = MeshComp->AnimClass;
		MeshComp->SetAnimInstanceClass(nullptr);

		FTimerHandle AnimResetTimer;
		GetWorld()->GetTimerManager().SetTimer(AnimResetTimer, [MeshComp, AnimClass]()
		{
			if (MeshComp && AnimClass)
			{
				MeshComp->SetAnimInstanceClass(AnimClass);
				MeshComp->InitializeAnimScriptInstance(true);
			}
		}, 0.1f, false);
	}
}
#pragma endregion

#pragma region Peaceful AI
void AAIMonsterBase::CallNearbyAllies(AActor* Enemy)
{
	if (!Enemy) return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == this) continue;

		AAIMonsterBase* Ally = Cast<AAIMonsterBase>(Actor);
		if (!Ally) continue;
		if (Ally->Personality != EMonsterPersonality::Peaceful) continue;
		if (Ally->bIsAggressive || Ally->IsDead()) continue;
		if (FVector::Dist(GetActorLocation(), Ally->GetActorLocation()) > AllyCallRadius) continue;

		Ally->bIsAggressive = true;
		Ally->Aggressor     = Enemy;
		Ally->RequestState(EMonsterState::Aggressive);

		if (AAIController* AllyAIC = Cast<AAIController>(Ally->GetController()))
		{
			if (UBlackboardComponent* BB = AllyAIC->GetBlackboardComponent())
			{
				if (!BB->GetValueAsObject(TEXT("TargetPlayer")))
					BB->SetValueAsObject(TEXT("TargetPlayer"), Enemy);
			}
			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AllyAIC->GetBrainComponent()))
			{
				BTComp->RestartTree();
			}
		}
	}
}

void AAIMonsterBase::ResetToPassive()
{
	bIsAggressive = false;
	bIsAttacking  = false;
	Aggressor     = nullptr;

	// GAS를 통한 체력 복원
	if (AbilitySystemComponent && AttributeSet)
	{
		const float MaxHP = AttributeSet->GetMaxHealth();
		AbilitySystemComponent->SetNumericAttributeBase(
			AttributeSet->GetHealthAttribute(), MaxHP
		);
	}

	RequestState(EMonsterState::Passive);
}
#pragma endregion

#pragma region HP Bar Widget
void AAIMonsterBase::ShowHPBar()
{
	if (!HPWidgetComponent) return;

	HPWidgetComponent->SetVisibility(true);
	if (UUserWidget* Widget = HPWidgetComponent->GetUserWidgetObject())
	{
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AAIMonsterBase::HideHPBar()
{
	if (!HPWidgetComponent) return;

	if (UUserWidget* Widget = HPWidgetComponent->GetUserWidgetObject())
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
	HPWidgetComponent->SetVisibility(false);
}

void AAIMonsterBase::UpdateHPBarWidget()
{
	if ( !HPWidgetComponent )
		return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if ( !PC )
		return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector WidgetLocation = HPWidgetComponent->GetComponentLocation();

	// 카메라 방향 계산
	FVector Direction = CameraLocation - WidgetLocation;

	// HP바가 기울어지지 않게 Z 제거
	Direction.Z = 0.f;

	FRotator LookAtRotation = Direction.Rotation();

	HPWidgetComponent->SetWorldRotation(LookAtRotation);

	// 크기 고정
	HPWidgetComponent->SetWorldScale3D(FVector(0.5f));
}

#pragma endregion

#pragma region Alert Icon
void AAIMonsterBase::ShowAlertIcon()
{
	if (bIsAlerting) return;
	if (!AlertWidgetComponent || !AlertWidget) return;

	bIsAlerting = true;

	AlertWidgetComponent->SetVisibility(true);
	AlertWidgetComponent->SetHiddenInGame(false);
	AlertWidget->SetVisibility(ESlateVisibility::Visible);
}

void AAIMonsterBase::HideAlertIcon()
{
	if (!bIsAlerting) return;
	if (!AlertWidgetComponent || !AlertWidget) return;

	bIsAlerting = false;

	AlertWidget->SetVisibility(ESlateVisibility::Collapsed);
	AlertWidgetComponent->SetVisibility(false);
	AlertWidgetComponent->SetHiddenInGame(true);
}
#pragma endregion	

#pragma region Rotation System
void AAIMonsterBase::StartRotationUpdate()
{
	if (!GetWorld()) return;
    
	if (GetWorldTimerManager().IsTimerActive(RotationTimerHandle))
		return;
    
	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&AAIMonsterBase::UpdateRotation,
		RotationUpdateInterval,
		true
	);
}

void AAIMonsterBase::StopRotationUpdate()
{
	if (!GetWorld()) return;
    
	if (GetWorldTimerManager().IsTimerActive(RotationTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(RotationTimerHandle);
	}
}

void AAIMonsterBase::UpdateRotation()
{
	if (!bUseSmoothRotation) return;

	// 이동 중이면 bOrientRotationToMovement가 자동 처리
	const float CurrentSpeed = GetVelocity().Size2D();
	if (CurrentSpeed > 10.f) return;

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC) return;

	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) return;

	FVector TargetLocation = FVector::ZeroVector;
	bool bHasTarget = false;

	// 정지 상태에서만 타겟을 부드럽게 바라봄
	if (CurrentState == EMonsterState::Alert || 
	    CurrentState == EMonsterState::Aggressive ||
	    CurrentState == EMonsterState::Attack)
	{
		AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer")));
		if (TargetActor)
		{
			TargetLocation = TargetActor->GetActorLocation();
			bHasTarget = true;
		}
	}

	if (!bHasTarget) return;

	const FVector CurrentLocation = GetActorLocation();
	const FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal2D();
	
	if (DirectionToTarget.IsNearlyZero()) return;

	const FRotator TargetRotation = DirectionToTarget.Rotation();
	const FRotator CurrentRotation = GetActorRotation();

	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		RotationUpdateInterval,
		RotationSpeed
	);

	SetActorRotation(NewRotation);
}
#pragma endregion

#pragma region DataTable
float AAIMonsterBase::CalculateExp(int32 PlayerLevel) const
{
	const FUK_MonsterStatRow* Row = GetStatRow();
	if (!Row) return 0.f;
	return Row->BaseExp + Row->ExpPerLevel * (PlayerLevel - 1);
}

float AAIMonsterBase::CalculateGold(int32 PlayerLevel) const
{
	const FUK_MonsterStatRow* Row = GetStatRow();
	if (!Row) return 0.f;
	return Row->BaseGold + Row->GoldPerLevel * (PlayerLevel - 1);
}

FName AAIMonsterBase::GetRowName() const
{
    const UEnum* Enum = StaticEnum<EMonsterType>();
    if (!Enum) return NAME_None;

    FString Full = Enum->GetNameStringByValue((int64)MonsterType);

    int32 ColonIdx;
    if (Full.FindLastChar(':', ColonIdx))
        return FName(*Full.Mid(ColonIdx + 1));

    return FName(*Full);
}

const FUK_MonsterStatRow* AAIMonsterBase::GetStatRow() const
{
    if (!MonsterStatTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] MonsterStatTable이 할당되지 않았습니다."), *GetName());
        return nullptr;
    }
    return MonsterStatTable->FindRow<FUK_MonsterStatRow>(GetRowName(), TEXT("GetStatRow"));
}

const FUK_MonsterMetaRow* AAIMonsterBase::GetMetaRow() const
{
    if (!MonsterMetaTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] MonsterMetaTable이 할당되지 않았습니다."), *GetName());
        return nullptr;
    }
    return MonsterMetaTable->FindRow<FUK_MonsterMetaRow>(GetRowName(), TEXT("GetMetaRow"));
}

// ────────────────────────────────────────────────
//  스탯 계산
// ────────────────────────────────────────────────

float AAIMonsterBase::CalculateMaxHealth(int32 PlayerLevel) const
{
    const FUK_MonsterStatRow* Row = GetStatRow();
    if (!Row) return 100.f;
    return Row->BaseHP + (PlayerLevel * Row->HPPerLevel);
}

float AAIMonsterBase::CalculateAttackDamage(int32 PlayerLevel) const
{
    const FUK_MonsterStatRow* Row = GetStatRow();
    if (!Row) return 0.f;
    return PlayerLevel * Row->AttackMultiplier;
}

float AAIMonsterBase::CalculateAoEDamage(int32 PlayerLevel) const
{
    const FUK_MonsterStatRow* Row = GetStatRow();
    if (!Row) return 0.f;
    return CalculateAttackDamage(PlayerLevel) * Row->AoEMultiplier;
}

float AAIMonsterBase::CalculateDefense(int32 PlayerLevel) const
{
    const FUK_MonsterStatRow* Row = GetStatRow();
    if (!Row) return 0.f;
    return (PlayerLevel / 2.f) + Row->BaseDefense;
}

// ────────────────────────────────────────────────
//  스탯 초기화
// ────────────────────────────────────────────────

void AAIMonsterBase::InitializeStatsFromPlayerLevel(int32 PlayerLevel)
{
    if (PlayerLevel <= 0) return;

    const FUK_MonsterStatRow* Row = GetStatRow();
    if (!Row) return;

    // 전투 파라미터 반영
    AttackDamage     = CalculateAttackDamage(PlayerLevel);
    AttackRange      = Row->AttackRange;
    AttackCooldown   = Row->AttackCooldown;
    DetectionRadius  = Row->DetectionRadius;
    MaxChaseDistance = Row->MaxChaseDistance;

    // GAS AttributeSet HP/Defense 반영
    if (AbilitySystemComponent && AttributeSet)
    {
        const float MaxHP     = CalculateMaxHealth(PlayerLevel);
        const float NewDefense = CalculateDefense(PlayerLevel);

        AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), MaxHP);
        AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetHealthAttribute(), MaxHP);
        AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetDefenseAttribute(), NewDefense);

        UE_LOG(LogTemp, Log,
            TEXT("[%s] InitStats | Lv=%d | HP=%.0f | ATK=%.1f | DEF=%.1f"),
            *GetName(), PlayerLevel, MaxHP, AttackDamage, NewDefense);
    }
}

// ────────────────────────────────────────────────
//  퀘스트 연동
// ────────────────────────────────────────────────

FString AAIMonsterBase::GetKillEventId() const
{
    const FUK_MonsterMetaRow* Meta = GetMetaRow();
    if (!Meta || Meta->MobEntityId.IsNone()) return FString();

    // QuestEvent.Killed.Mob_Common_Wolf
    return FString::Printf(TEXT("QuestEvent.Killed.%s"), *Meta->MobEntityId.ToString());
}

void AAIMonsterBase::AutoInitStatsFromNearestPlayer()
{
	if (!GetWorld()) return;

	int32 BestLevel  = 1;
	float BestDistSq = FLT_MAX;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AUK_CharacterBase* PlayerChar = Cast<AUK_CharacterBase>(PC->GetPawn());
		if (!PlayerChar) continue;

		const float DistSq = FVector::DistSquared(GetActorLocation(), PlayerChar->GetActorLocation());
		if (DistSq >= BestDistSq) continue;

		BestDistSq = DistSq;

		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(PlayerChar))
		{
			if (UAbilitySystemComponent* PlayerASC = ASCInterface->GetAbilitySystemComponent())
			{
				if (const UUK_PlayerStatusAttributeSet* PlayerAttr =
					PlayerASC->GetSet<UUK_PlayerStatusAttributeSet>())
				{
					BestLevel = FMath::Max(1, FMath::RoundToInt(PlayerAttr->GetLevel()));
				}
			}
		}
	}

	InitializeStatsFromPlayerLevel(BestLevel);
}

void AAIMonsterBase::GrantRewardsToKiller()
{
	if (bRewardGranted) return;
	bRewardGranted = true;
	
	if (!LastAttackerController)
	{
		APlayerController* FallbackPC = GetWorld()->GetFirstPlayerController();
		LastAttackerController = FallbackPC;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Reward] GrantRewardsToKiller - Controller: %s"),
		LastAttackerController ? *LastAttackerController->GetName() : TEXT("NULL"));

	if (!LastAttackerController) return;

    APawn* KillerPawn = LastAttackerController->GetPawn();
    if (!KillerPawn) return;

    int32 PlayerLevel = 1;
    float ExpGain = 0.f;
    int32 GoldGain = 0;

    // ── 경험치 ──────────────────────────────────────────
    if (IAbilitySystemInterface* ASIPlayer = Cast<IAbilitySystemInterface>(KillerPawn))
    {
        if (UAbilitySystemComponent* PlayerASC = ASIPlayer->GetAbilitySystemComponent())
        {
            PlayerLevel = FMath::Max(1, FMath::RoundToInt(
                PlayerASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetLevelAttribute())));

            const float CurrentExp = PlayerASC->GetNumericAttribute(
                UUK_PlayerStatusAttributeSet::GetEXPAttribute());
            ExpGain = CalculateExp(PlayerLevel);
            PlayerASC->SetNumericAttributeBase(
                UUK_PlayerStatusAttributeSet::GetEXPAttribute(), CurrentExp + ExpGain);
        }
    }

    // ── 아이템 드롭 ──────────────────────────────────────
    UUK_InventoryComponent* Inventory = KillerPawn->FindComponentByClass<UUK_InventoryComponent>();
    TArray<FString> DroppedItems;

    if (MonsterLootTable && Inventory)
    {
        const FUK_MonsterLootRow* LootRow = MonsterLootTable->FindRow<FUK_MonsterLootRow>(
            GetRowName(), TEXT("AAIMonsterBase::GrantRewardsToKiller"));

        if (LootRow)
        {
            for (const FUK_MonsterLootEntry& Entry : LootRow->Entries)
            {
                if (Entry.ItemId == NAME_None) continue;
                if (FMath::FRand() > Entry.DropChance) continue;

                const int32 Count = FMath::RandRange(Entry.MinCount, Entry.MaxCount);
                Inventory->AddItem(Entry.ItemId, Count);
                DroppedItems.Add(FString::Printf(TEXT("%s x%d"), *Entry.ItemId.ToString(), Count));
            }
        }
    }
	
	// ── Gold ────────────────────────────────────────────
	if (Inventory)
	{
		GoldGain = FMath::RoundToInt(CalculateGold(PlayerLevel));
		Inventory->AddGold(GoldGain);
	}

    // ── 로그 ─────────────────────────────────────────────
    UE_LOG(LogTemp, Warning, TEXT("========= [Monster Killed: %s] ========="), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  Player Level : %d"), PlayerLevel);
	UE_LOG(LogTemp, Warning, TEXT("  Gold Gained  : %d"), GoldGain);
    UE_LOG(LogTemp, Warning, TEXT("  EXP Gained   : %.1f"), ExpGain);

    if (DroppedItems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Items        : None"));
    }
    else
    {
        for (const FString& ItemLog : DroppedItems)
        {
            UE_LOG(LogTemp, Warning, TEXT("  Item Drop    : %s"), *ItemLog);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("========================================="));
}
#pragma endregion

void AAIMonsterBase::PlayHitEffect(FVector ImpactPoint)
{
	if (HitEffect)
	{
		FVector SpawnLoc = (ImpactPoint.IsNearlyZero()) ? GetActorLocation() + FVector(0.f, 0.f, 100.f) : ImpactPoint;
		
		FVector EffectScale = FVector(2.5f, 2.5f, 2.5f); 

		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), 
			HitEffect, 
			SpawnLoc, 
			FRotator::ZeroRotator,
			EffectScale,          
			true                 
		);
	}
}