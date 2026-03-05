#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/UK_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Quest/UKQuestManagerSubsystem.h"

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
		GetCharacterMovement()->RotationRate                  = FRotator(0.f, 540.f, 0.f);
	}

	// HP Widget 설치
	HPWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidgetComponent"));
	HPWidgetComponent->SetupAttachment(GetMesh());
	HPWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPWidgetComponent->SetDrawAtDesiredSize(true);
	HPWidgetComponent->SetDrawSize(FVector2D(180.f, 20.f));
	HPWidgetComponent->SetRelativeLocation(FVector(0, 0, 120.f));
	HPWidgetComponent->SetVisibility(false);
	
	// Alert Icon Widget 설치
	AlertWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AlertWidgetComponent"));
	AlertWidgetComponent->SetupAttachment(GetMesh());
	AlertWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);   
	AlertWidgetComponent->SetDrawAtDesiredSize(true);
	AlertWidgetComponent->SetDrawSize(FVector2D(64.f, 64.f));
	AlertWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 160.f)); 
	AlertWidgetComponent->SetVisibility(false);
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

	if (HPWidgetComponent)
	{
		GetWorldTimerManager().SetTimer(
			HPBarUpdateTimer, this,
			&AAIMonsterBase::UpdateHPBarWidget,
			0.05f, true);
	}
}

void AAIMonsterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HPWidgetComponent && HPWidgetClass)
	{
		HPWidgetComponent->SetWidgetClass(HPWidgetClass);

		if ( UUK_MonsterHealthBar* Widget =
			Cast<UUK_MonsterHealthBar>(HPWidgetComponent->GetUserWidgetObject()) )
		{

		if (HPWidget && AbilitySystemComponent && AttributeSet)
		{
			HPWidget->BindMonsterAttributes(AbilitySystemComponent, AttributeSet);
		}
		}
	
		if (AlertWidgetComponent && AlertWidgetClass)
		{
			AlertWidgetComponent->SetWidgetClass(AlertWidgetClass);
		}
	}
}

UAbilitySystemComponent* AAIMonsterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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

	OnStateChanged.Broadcast(OldState, NewState);
}
#pragma endregion

#pragma region State Callbacks
void AAIMonsterBase::SetAIActive(bool bActive) {}
void AAIMonsterBase::OnIdle()                  {}
void AAIMonsterBase::OnPatrol()                {}
void AAIMonsterBase::OnChase(float DeltaSeconds) {}
void AAIMonsterBase::OnDead()                  {}
void AAIMonsterBase::OnPassive()               {}
void AAIMonsterBase::OnAlert()                 {}

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
	EffectContext.AddInstigator(InstigatorController ? InstigatorController->GetPawn() : nullptr, this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectClass, 1.0f, EffectContext);

	if (SpecHandle.IsValid())
	{
		// Damage 값 설정
		FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
		
		if (!DamageTag.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] ✗✗✗ CRITICAL: GameplayTag 'Data.Damage' NOT REGISTERED!"), 
				*GetName());
			UE_LOG(LogTemp, Error, TEXT("   → Project Settings → GameplayTags에서 Data.Damage 추가 필요"));
			return;
		}
		
		SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, DamageAmount);
		
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
		AActor* ClosestPlayer = nullptr;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;

			if (FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation()) <= 500.0f)
			{
				ClosestPlayer = PC->GetPawn();
				break;
			}
		}

		if (ClosestPlayer)
		{
			bIsAggressive = true;
			Aggressor      = ClosestPlayer;

			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					if (!BB->GetValueAsObject(TEXT("TargetPlayer")))
						BB->SetValueAsObject(TEXT("PendingTarget"), ClosestPlayer);
				}
			}

			CallNearbyAllies(ClosestPlayer);
			OnAttacked.Broadcast(this, ClosestPlayer);
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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(AttackMontages[MontageIndex]);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AAIMonsterBase::OnAttackMontageEnded);
}

void AAIMonsterBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	OnAttackFinished.ExecuteIfBound(!bInterrupted);
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

UUK_MonsterHealthBar* AAIMonsterBase::GetHPWidget() const
{
	if ( !HPWidgetComponent ) return nullptr;

	return Cast<UUK_MonsterHealthBar>(
		HPWidgetComponent->GetUserWidgetObject()
	);
}

//void AAIMonsterBase::UpdateHPBarWidget()
//{
//	if ( !HPWidgetComponent || !HPWidgetComponent->IsVisible() ) return;
//
//	// ----- 카메라 위치 얻기 -----
//	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
//	if ( !PC || !PC->PlayerCameraManager ) return;
//
//	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
//	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
//
//	// ----- 카메라를 바라보게 회전 -----
//	const FVector WidgetLocation = HPWidgetComponent->GetComponentLocation();
//	float Distance = FVector::Dist(CameraLocation, WidgetLocation);
//
//	// ----- Pitch, Roll 제거 ------
//	FRotator NewRotation = CameraRotation;
//	NewRotation.Yaw += 180.f;
//	NewRotation.Roll = 0.f;
//
//	HPWidgetComponent->SetWorldRotation(NewRotation);
//
//	// --- 화면상 HP UI 크기 유지용 스케일 ---
//	float ScaleFactor = FMath::Max(0.1f, Distance / 2000.f);
//	HPWidgetComponent->SetWorldScale3D(FVector(DesiredScale));
//
//}

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

//void AAIMonsterBase::UpdateHPBarWidget()
//{
//	if ( !HPWidgetComponent ) return;
//
//	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
//	if ( !PC ) return;
//
//	// ----- 카메라 위치 얻기 -----
//	FVector CameraLocation;
//	FRotator CameraRotation;
//	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
//
//	// ----- 위젯 위치 -----
//	const FVector WidgetLocation = HPWidgetComponent->GetComponentLocation();
//
//	// ----- 카메라를 바라보게 회전 -----
//	FVector Direction = CameraLocation - WidgetLocation;
//	FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
//
//	// ----- Pitch, Roll 제거 ------
//	LookAtRotation.Pitch = 0.f;
//	LookAtRotation.Roll = 0.f;
//
//	HPWidgetComponent->SetWorldRotation(LookAtRotation);
//
//	// --- 화면상 HP UI 크기 유지용 스케일 ---
//	const FVector DesiredScale(0.5f, 0.5f, 0.5f);
//	HPWidgetComponent->SetWorldScale3D(DesiredScale);
//}

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

	// AI 정지
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}

	SetState(EMonsterState::Dead);

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

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAIMonsterBase::HideAndBroadcastDeath()
{
	HideCorpse();

	NotifyMonsterKilled();
	OnDeath.Broadcast(this);
}

static FName GetMonsterEventId(EMonsterType Type)
{
	switch (Type)
	{
	case EMonsterType::Wolf:       return FName("QuestEvent.Killed.Mob_Common_Wolf");
	case EMonsterType::Fox:        return FName("QuestEvent.Killed.Mob_Common_Fox");
	case EMonsterType::Reindeer:   return FName("QuestEvent.Killed.Mob_Common_Reindeer");
	case EMonsterType::Golem:      return FName("QuestEvent.Killed.Mob_Common_Golem");
	case EMonsterType::EliteGolem: return FName("QuestEvent.Killed.Mob_Common_EliteGolem");
	default: return NAME_None;
	}
}

void AAIMonsterBase::NotifyMonsterKilled()
{
	OnMonsterKilled.Broadcast(this, MonsterType, LastAttackerController);

	// 퀘스트 이벤트
	//const FName EventId = GetMonsterEventId(MonsterType);
	//if (EventId == NAME_None) return;

	//if (UUKQuestManagerSubsystem* QM = GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>())
	//{
	//	QM->EmitQuestEvent(EventId);
	//}
}

void AAIMonsterBase::HideCorpse()
{
	SetActorHiddenInGame(true);
}
#pragma endregion

#pragma region Respawn
void AAIMonsterBase::ResetHealth()
{
	GetWorldTimerManager().ClearTimer(CorpseTimerHandle);

	// 플래그 리셋
	bIsAttacking           = false;
	bIsDying               = false;
	LastAttackerController = nullptr;

	// GAS를 통한 체력 복원
	if (AbilitySystemComponent && AttributeSet)
	{
		const float MaxHP = AttributeSet->GetMaxHealth();
		AbilitySystemComponent->SetNumericAttributeBase(
			AttributeSet->GetHealthAttribute(), MaxHP
		);
	}

	// 상태 복원
	if (Personality == EMonsterPersonality::Peaceful)
	{
		bIsAggressive = false;
		Aggressor     = nullptr;
		SetState(EMonsterState::Passive);
	}
	else
	{
		SetState(EMonsterState::Idle);
	}

	// 충돌 / 이동 복원
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->StopMovementImmediately();
	}

	// 위치 초기화
	SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	SetActorRotation(FRotator::ZeroRotator);

	// 외형 + 애니메이션 리셋
	ResetAppearance();

	// AI 재시작
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsVector(TEXT("SpawnLocation"), SpawnLocation);
			BB->SetValueAsVector(TEXT("PatrolLocation"), SpawnLocation);
			BB->ClearValue(TEXT("TargetPlayer"));
		}

		if (BehaviorTree)
		{
			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
			{
				BTComp->StopTree();

				FTimerHandle RestartTimer;
				GetWorldTimerManager().SetTimer(RestartTimer, [this, AIC]()
				{
					if (BehaviorTree && AIC && AIC->GetBrainComponent())
					{
						if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
						{
							BT->StartTree(*BehaviorTree);
						}
					}
				}, 0.5f, false);
			}
		}
	}
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
					BB->SetValueAsObject(TEXT("PendingTarget"), Enemy);
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
	if (!HPWidgetComponent) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	FVector  CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector WidgetLocation = HPWidgetComponent->GetComponentLocation();
	FVector  Direction           = CameraLocation - WidgetLocation;
	FRotator LookAtRotation      = FRotationMatrix::MakeFromX(Direction).Rotator();

	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Roll  = 0.f;
	HPWidgetComponent->SetWorldRotation(LookAtRotation);

	HPWidgetComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
}
#pragma endregion

#pragma region Alert Icon
void AAIMonsterBase::ShowAlertIcon()
{
	if (!AlertWidgetComponent) return;

	if (!AlertWidgetComponent->GetUserWidgetObject() && AlertWidgetClass)
	{
		AlertWidgetComponent->SetWidgetClass(AlertWidgetClass);
		AlertWidgetComponent->InitWidget();
	}

	AlertWidgetComponent->SetVisibility(true);

	if (UUserWidget* W = AlertWidgetComponent->GetUserWidgetObject())
		W->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void AAIMonsterBase::HideAlertIcon()
{
	if (!AlertWidgetComponent) return;

	if (UUserWidget* W = AlertWidgetComponent->GetUserWidgetObject())
		W->SetVisibility(ESlateVisibility::Collapsed);

	AlertWidgetComponent->SetVisibility(false);
}
#pragma endregion