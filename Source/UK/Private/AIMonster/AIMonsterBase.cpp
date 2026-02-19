#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/UK_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAIMonsterBase::AAIMonsterBase()
{
	PrimaryActorTick.bCanEverTick = false; 

	bReplicates = true;
	SetReplicateMovement(true);
	StatComponent = CreateDefaultSubobject<UAI_MonsterStatComponent>(TEXT("StatComponent"));
	NetDormancy = DORM_DormantAll;
	
	bUseControllerRotationYaw = false;
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
		
		GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.15f;
		GetCharacterMovement()->NetworkSimulatedSmoothRotationTime = 0.15f;
		GetCharacterMovement()->ListenServerNetworkSimulatedSmoothLocationTime = 0.15f;
		GetCharacterMovement()->ListenServerNetworkSimulatedSmoothRotationTime = 0.15f;
		GetCharacterMovement()->NetworkMaxSmoothUpdateDistance = 256.f;
		GetCharacterMovement()->NetworkNoSmoothUpdateDistance = 512.f;
	}
}

void AAIMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnLocation = GetActorLocation();

		if (Personality == EMonsterPersonality::Peaceful)
		{
			CurrentState = EMonsterState::Passive;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("🟢 [BeginPlay] %s: Initialized at %s"), 
			*GetName(), *SpawnLocation.ToString());
	}
}

void AAIMonsterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// ★★★ 델리게이트는 여기서 한 번만 바인딩
	if (HasAuthority() && StatComponent)
	{
		StatComponent->OnDeath.RemoveAll(this);
		StatComponent->OnDeath.AddDynamic(this, &AAIMonsterBase::Die);
		
		UE_LOG(LogTemp, Warning, TEXT("🔗 [PostInit] %s: OnDeath delegate bound"), *GetName());
	}
}

void AAIMonsterBase::RequestState_Implementation(EMonsterState NewState)
{
	if (!HasAuthority()) return;
	if (CurrentState == NewState) return;
	SetServerState(NewState);
}

void AAIMonsterBase::SetServerState(EMonsterState NewState)
{
	EMonsterState OldState = CurrentState;
	CurrentState = NewState;

	OnStateChanged.Broadcast(OldState, NewState);
	OnRep_MonsterState();
}

void AAIMonsterBase::OnRep_MonsterState()
{
	switch (CurrentState)
	{
	case EMonsterState::Idle:
	case EMonsterState::Passive:
		SetNetDormancy(DORM_DormantAll);
		break;
	case EMonsterState::Dead:
		break;
	default:
		SetNetDormancy(DORM_Awake);
		break;
	}
}

void AAIMonsterBase::OnRep_IsAggressive()
{
	UE_LOG(LogTemp, Log, TEXT("[OnRep] %s bIsAggressive = %d"), *GetName(), bIsAggressive);
}

bool AAIMonsterBase::PlayRandomAttackMontage()
{
	if (bIsAttacking || bIsDying) return false;
	if (!HasAuthority()) return false;

	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return false;

	if (AttackMontages.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack] %s: AttackMontages is empty!"), *GetName());
		return false;
	}

	int32 RandomIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
	UAnimMontage* SelectedMontage = AttackMontages[RandomIndex];

	if (!SelectedMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack] %s: Montage[%d] is null!"), *GetName(), RandomIndex);
		return false;
	}

	bIsAttacking = true;
	LastAttackTime = Now;

	Multicast_PlayAttackMontage(RandomIndex);
	
	return true;
}

void AAIMonsterBase::Multicast_PlayAttackMontage_Implementation(int32 MontageIndex)
{
	if (!AttackMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = AttackMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	float Length = AnimInstance->Montage_Play(Montage, 1.0f);
	
	if (HasAuthority() && Length > 0.f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Attack] %s: Playing attack montage[%d] on %s"), 
		*GetName(), MontageIndex, HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void AAIMonsterBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	OnAttackFinished.ExecuteIfBound(!bInterrupted);
}

void AAIMonsterBase::Die()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ [Die] %s: Called on client!"), *GetName());
		return;
	}
	
	// ★★★ 중복 호출 완전 차단
	if (bIsDying)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ [Die] %s: ALREADY DYING! Duplicate call BLOCKED!"), *GetName());
		return;
	}
	
	bIsDying = true;
	
	UE_LOG(LogTemp, Warning, TEXT("💀💀💀 [Die] %s: Die() CALLED 💀💀💀"), *GetName());

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

	// 상태 변경
	SetServerState(EMonsterState::Dead);

	// 사망 몽타주 재생
	if (DeathMontage)
	{
		Multicast_PlayDeathMontage();
		
		float MontageLength = DeathMontage->GetPlayLength();
		GetWorldTimerManager().SetTimer(
			DeathMontageTimerHandle, this, &AAIMonsterBase::FinalizeDeath,
			MontageLength, false);
			
		UE_LOG(LogTemp, Warning, TEXT("💀 [Die] %s: Death montage timer set (%.2fs)"), *GetName(), MontageLength);
	}
	else
	{
		FTimerHandle DeathTimer;
		GetWorldTimerManager().SetTimer(
			DeathTimer, this, &AAIMonsterBase::FinalizeDeath,
			DeathWithoutMontageDelay, false);
	}
}

void AAIMonsterBase::Multicast_PlayDeathMontage_Implementation()
{
	if (!DeathMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	// ★★★ 중복 재생 완전 차단
	if (AnimInstance->Montage_IsPlaying(DeathMontage))
	{
		UE_LOG(LogTemp, Error, TEXT("❌❌❌ [Death] %s: Death montage ALREADY PLAYING! BLOCKED!"), *GetName());
		return;
	}

	AnimInstance->StopAllMontages(0.2f);
	float Length = AnimInstance->Montage_Play(DeathMontage, 1.0f);
	
	UE_LOG(LogTemp, Warning, TEXT("💀 [Death] %s: Death montage STARTED (%.2fs) on %s"), 
		*GetName(), Length, HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void AAIMonsterBase::FinalizeDeath()
{
	if (!HasAuthority()) return;
	
	UE_LOG(LogTemp, Warning, TEXT("💀 [FinalizeDeath] %s: Starting"), *GetName());
	
	GetWorldTimerManager().ClearTimer(DeathMontageTimerHandle);
	
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
			CorpseTimerHandle, this, &AAIMonsterBase::HideAndBroadcastDeath,
			CorpseLingerTime, false);
			
		UE_LOG(LogTemp, Warning, TEXT("💀 [FinalizeDeath] %s: Corpse will linger %.1fs"), 
			*GetName(), CorpseLingerTime);
	}
	else
	{
		HideAndBroadcastDeath();
	}
}

void AAIMonsterBase::HideAndBroadcastDeath()
{
	if (!HasAuthority()) return;
	
	UE_LOG(LogTemp, Warning, TEXT("💀 [HideAndBroadcast] %s: Hiding and broadcasting"), *GetName());
	
	Multicast_HideCorpse();
	SetNetDormancy(DORM_DormantAll);

	OnDeath.Broadcast(this);
}

void AAIMonsterBase::Multicast_HideCorpse_Implementation()
{
	SetActorHiddenInGame(true);
	UE_LOG(LogTemp, Log, TEXT("👻 [HideCorpse] %s: Hidden"), *GetName());
}

void AAIMonsterBase::ResetHealth()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("🔄🔄🔄 [ResetHealth] %s: ========== START ========== 🔄🔄🔄"), *GetName());

	// 1) 타이머 정리
	GetWorldTimerManager().ClearTimer(CorpseTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathMontageTimerHandle);
	UE_LOG(LogTemp, Log, TEXT("🔄 Step 1: Timers cleared"));

	// 2) ★★★ 플래그 리셋 (가장 먼저!)
	bIsAttacking = false;
	bIsDying = false;
	UE_LOG(LogTemp, Warning, TEXT("🔄 Step 2: FLAGS RESET (bIsDying=false)"));

	// 3) Stat 복원
	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
		UE_LOG(LogTemp, Log, TEXT("🔄 Step 3: HP restored to %.1f"), StatComponent->GetMaxHP());
	}

	// 4) 상태 복원
	if (Personality == EMonsterPersonality::Peaceful)
	{
		bIsAggressive = false;
		Aggressor = nullptr;
		SetServerState(EMonsterState::Passive);
		UE_LOG(LogTemp, Log, TEXT("🔄 Step 4: State → Passive"));
	}
	else
	{
		SetServerState(EMonsterState::Idle);
		UE_LOG(LogTemp, Log, TEXT("🔄 Step 4: State → Idle"));
	}

	// 5) 충돌/이동 복원
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->StopMovementImmediately();
	}
	UE_LOG(LogTemp, Log, TEXT("🔄 Step 5: Collision/Movement restored"));

	// 6) 위치 초기화
	SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	SetActorRotation(FRotator::ZeroRotator);
	UE_LOG(LogTemp, Log, TEXT("🔄 Step 6: Position reset to %s"), *SpawnLocation.ToString());

	// 7) Dormancy 해제
	FlushNetDormancy();
	SetNetDormancy(DORM_Awake);
	UE_LOG(LogTemp, Log, TEXT("🔄 Step 7: Dormancy flushed"));

	// 8) ★★★ 외형 + 애니메이션 완전 리셋
	Multicast_ResetAppearance();
	UE_LOG(LogTemp, Log, TEXT("🔄 Step 8: Multicast_ResetAppearance called"));

	// 9) AI 재시작 (긴 딜레이)
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
				GetWorldTimerManager().SetTimer(
					RestartTimer, 
					[this, AIC]()
					{
						if (BehaviorTree && AIC && AIC->GetBrainComponent())
						{
							if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
							{
								BTComp->StartTree(*BehaviorTree);
								UE_LOG(LogTemp, Warning, TEXT("🔄 Step 9: BT RESTARTED!"));
							}
						}
					},
					0.5f, false);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("🔄🔄🔄 [ResetHealth] %s: ========== COMPLETE ========== 🔄🔄🔄"), *GetName());
}

void AAIMonsterBase::Multicast_ResetAppearance_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("🎭🎭🎭 [ResetAppearance] %s: CALLED 🎭🎭🎭"), *GetName());
	
	// 1) 보이기
	SetActorHiddenInGame(false);

	// 2) ★★★ 애니메이션만 리셋 (메시 트랜스폼은 건드리지 않음!)
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		// 물리 끄기
		MeshComp->SetAllBodiesSimulatePhysics(false);
		
		UE_LOG(LogTemp, Log, TEXT("🎭 Mesh physics disabled"));
	}

	// 3) ★★★ 애니메이션 완전 리셋
	UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		// 모든 몽타주 즉시 정지
		AnimInstance->Montage_Stop(0.f);
		AnimInstance->StopAllMontages(0.f);
		
		// 현재 재생 중인 몽타주도 강제 정지
		if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
		{
			AnimInstance->Montage_Stop(0.f, CurrentMontage);
			UE_LOG(LogTemp, Log, TEXT("🎭 Stopped active montage: %s"), *CurrentMontage->GetName());
		}
		
		UE_LOG(LogTemp, Warning, TEXT("🎭 All montages stopped"));
	}

	// 4) ★★★ 애니메이션 인스턴스 재생성 (메시 트랜스폼 보존)
	if (MeshComp && MeshComp->AnimClass)
	{
		UClass* AnimClass = MeshComp->AnimClass;
		
		// 기존 인스턴스 제거
		MeshComp->SetAnimInstanceClass(nullptr);
		
		// 짧은 딜레이 후 재생성
		FTimerHandle AnimResetTimer;
		GetWorld()->GetTimerManager().SetTimer(
			AnimResetTimer,
			[this, MeshComp, AnimClass]()
			{
				if (MeshComp && AnimClass)
				{
					MeshComp->SetAnimInstanceClass(AnimClass);
					MeshComp->InitializeAnimScriptInstance(true);
					
					UE_LOG(LogTemp, Warning, TEXT("🎭🎭🎭 AnimInstance RECREATED! 🎭🎭🎭"));
				}
			},
			0.1f, false
		);
	}
}

void AAIMonsterBase::CallNearbyAllies(AActor* Enemy)
{
	if (!HasAuthority() || !Enemy) return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == this) continue;

		AAIMonsterBase* Ally = Cast<AAIMonsterBase>(Actor);
		if (!Ally) continue;
		if (Ally->Personality != EMonsterPersonality::Peaceful) continue;
		if (Ally->bIsAggressive || Ally->IsDead()) continue;

		float Distance = FVector::Dist(GetActorLocation(), Ally->GetActorLocation());
		if (Distance > AllyCallRadius) continue;

		Ally->bIsAggressive = true;
		Ally->Aggressor = Enemy;
		Ally->RequestState(EMonsterState::Aggressive);

		if (AAIController* AllyAIC = Cast<AAIController>(Ally->GetController()))
		{
			if (UBlackboardComponent* BB = AllyAIC->GetBlackboardComponent())
			{
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
	if (!HasAuthority()) return;

	bIsAggressive = false;
	bIsAttacking = false;
	Aggressor = nullptr;

	RequestState(EMonsterState::Passive);
	
	UE_LOG(LogTemp, Warning, TEXT("[ResetToPassive] %s: Reset to passive"), *GetName());
}

void AAIMonsterBase::SetAIActive(bool bActive) {}

void AAIMonsterBase::ReceiveDamage(float Damage)
{
	if (!HasAuthority()) return;
	if (bIsDying)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ [ReceiveDamage] %s: Already dying, ignoring damage"), *GetName());
		return;
	}

	if (StatComponent)
	{
		float BeforeHp = StatComponent->GetHP();
		StatComponent->TakeDamage(Damage);

		UE_LOG(LogTemp, Warning,
			TEXT("[Monster Hit] %s | Dmg: %.1f | HP: %.1f → %.1f"),
			*GetName(), Damage, BeforeHp, StatComponent->GetHP());
	}

	if (Personality == EMonsterPersonality::Peaceful && !bIsAggressive)
	{
		AActor* ClosestPlayer = nullptr;
		
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;

			float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Dist <= 500.0f)
			{
				ClosestPlayer = PC->GetPawn();
				break;
			}
		}

		if (ClosestPlayer)
		{
			bIsAggressive = true;
			Aggressor = ClosestPlayer;

			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					BB->SetValueAsObject(TEXT("TargetPlayer"), ClosestPlayer);
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

void AAIMonsterBase::OnIdle() {}
void AAIMonsterBase::OnPatrol() {}
void AAIMonsterBase::OnChase(float DeltaSeconds) {}

void AAIMonsterBase::OnAttack()
{
	if (!HasAuthority()) return;
	PlayRandomAttackMontage();
}

void AAIMonsterBase::OnDead() {}
void AAIMonsterBase::OnPassive() {}
void AAIMonsterBase::OnAlert() {}

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAIMonsterBase, CurrentState);
	DOREPLIFETIME(AAIMonsterBase, bIsAggressive);
	DOREPLIFETIME(AAIMonsterBase, Aggressor);
}