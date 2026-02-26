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

	//HP Widget 설치
	HPWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidgetComponent"));
	HPWidgetComponent->SetupAttachment(GetMesh());

	HPWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPWidgetComponent->SetDrawAtDesiredSize(true);
	//HPWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPWidgetComponent->SetDrawSize(FVector2D(180.f, 20.f));
	HPWidgetComponent->SetRelativeLocation(FVector(0, 0, 120.f));
	HPWidgetComponent->SetVisibility(false);
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
		
	}

	if ( HPWidgetComponent )
	{
		// 0.05초마다 HP바 갱신
		GetWorldTimerManager().SetTimer(
			HPBarUpdateTimer,
			this,
			&AAIMonsterBase::UpdateHPBarWidget,
			0.05f,
			true
		);
	}
}

void AAIMonsterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority() && StatComponent)
	{
		StatComponent->OnDeath.RemoveAll(this);
		StatComponent->OnDeath.AddDynamic(this, &AAIMonsterBase::Die);
		
		UE_LOG(LogTemp, Warning, TEXT("[PostInit] %s: OnDeath delegate bound"), *GetName());
	}

	if ( HPWidgetComponent && HPWidgetClass )
	{
		HPWidgetComponent->SetWidgetClass(HPWidgetClass);

		HPWidget = Cast<UUK_MonsterHealthBar>(HPWidgetComponent->GetUserWidgetObject());

		if ( HPWidget && StatComponent )
		{
			HPWidget->BindMonsterStats(StatComponent);
		}
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
		UE_LOG(LogTemp, Error, TEXT("[Die] %s: Called on client!"), *GetName());
		return;
	}
	
	if (bIsDying)
	{
		UE_LOG(LogTemp, Error, TEXT("[Die] %s: ALREADY DYING! Duplicate call BLOCKED!"), *GetName());
		return;
	}
	
	bIsDying = true;
	
	UE_LOG(LogTemp, Warning, TEXT("[Die] %s (Type=%d): Die() CALLED"), 
		*GetName(), (int32)MonsterType);

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

	// 고정 딜레이 후 FinalizeDeath
	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AAIMonsterBase::FinalizeDeath, CorpseLingerTime > 0.f ? 1.0f : 0.1f, false);

}


void AAIMonsterBase::FinalizeDeath()
{
	if (!HasAuthority()) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[FinalizeDeath] %s: Starting"), *GetName());
	
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
			
		UE_LOG(LogTemp, Warning, TEXT("[FinalizeDeath] %s: Corpse will linger %.1fs"), 
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
	
	UE_LOG(LogTemp, Warning, TEXT("[HideAndBroadcast] %s: Hiding and broadcasting"), *GetName());
	
	Multicast_HideCorpse();
	SetNetDormancy(DORM_DormantAll);

	// 킬 알림 전송 (서버 → GameMode)
	NotifyMonsterKilled();

	OnDeath.Broadcast(this);
}

void AAIMonsterBase::NotifyMonsterKilled()
{
	if (!HasAuthority()) return;

	// GameMode나 다른 시스템이 이걸 받아서 보상 처리
	OnMonsterKilled.Broadcast(this, MonsterType, LastAttackerController);
	
	UE_LOG(LogTemp, Error, TEXT("[Kill] %s (Type=%d) killed by %s"), 
		*GetName(), 
		(int32)MonsterType,
		LastAttackerController ? *LastAttackerController->GetName() : TEXT("UNKNOWN"));
}

//void AAIMonsterBase::UpdateHPBarScale(float NewScale)
//{
//	if ( HPWidgetComponent )
//	{
//		// NewScale 값에 따라 위젯의 물리적 크기를 변경
//		HPWidgetComponent->SetRelativeScale3D(FVector(NewScale));
//	}
//}

void AAIMonsterBase::ShowHPBar()
{
	if ( !HPWidgetComponent ) return;

	HPWidgetComponent->SetVisibility(true);

	if ( UUserWidget* Widget = HPWidgetComponent->GetUserWidgetObject() )
	{
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AAIMonsterBase::HideHPBar()
{
	if ( !HPWidgetComponent ) return;

	if ( UUserWidget* Widget = HPWidgetComponent->GetUserWidgetObject() )
	{
		// 핵심 ⭐
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}

	HPWidgetComponent->SetVisibility(false);
}

void AAIMonsterBase::Multicast_HideCorpse_Implementation()
{
	SetActorHiddenInGame(true);
	UE_LOG(LogTemp, Log, TEXT("[HideCorpse] %s: Hidden"), *GetName());
}

void AAIMonsterBase::ResetHealth()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(CorpseTimerHandle);

	// 플래그 리셋
	bIsAttacking = false;
	bIsDying = false;
	LastAttackerController = nullptr; 

	// Stat 복원
	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
	}

	// 상태 복원
	if (Personality == EMonsterPersonality::Peaceful)
	{
		bIsAggressive = false;
		Aggressor = nullptr;
		SetServerState(EMonsterState::Passive);
	}
	else
	{
		SetServerState(EMonsterState::Idle);
	}

	// 충돌/이동 복원
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

	// Dormancy 해제
	FlushNetDormancy();
	SetNetDormancy(DORM_Awake);

	// 외형 + 애니메이션 완전 리셋
	Multicast_ResetAppearance();

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
				GetWorldTimerManager().SetTimer(
					RestartTimer, 
					[this, AIC]()
					{
						if (BehaviorTree && AIC && AIC->GetBrainComponent())
						{
							if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
							{
								BTComp->StartTree(*BehaviorTree);
							}
						}
					},
					0.5f, false);
			}
		}
	}
}

void AAIMonsterBase::Multicast_ResetAppearance_Implementation()
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
		
		if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
		{
			AnimInstance->Montage_Stop(0.f, CurrentMontage);
		}
	}

	if (MeshComp && MeshComp->AnimClass)
	{
		UClass* AnimClass = MeshComp->AnimClass;
		MeshComp->SetAnimInstanceClass(nullptr);
		
		FTimerHandle AnimResetTimer;
		GetWorld()->GetTimerManager().SetTimer(
			AnimResetTimer,
			[this, MeshComp, AnimClass]()
			{
				if (MeshComp && AnimClass)
				{
					MeshComp->SetAnimInstanceClass(AnimClass);
					MeshComp->InitializeAnimScriptInstance(true);
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
	
	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
		UE_LOG(LogTemp, Warning, TEXT("[ResetToPassive] %s: Health restored to Max"), *GetName());
	}

	RequestState(EMonsterState::Passive);
	
	UE_LOG(LogTemp, Warning, TEXT("[ResetToPassive] %s: Reset to passive"), *GetName());
}

void AAIMonsterBase::SetAIActive(bool bActive) {}

bool AAIMonsterBase::PlayRandomIdleMontage()
{
	if (IdleMontages.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Idle] %s: IdleMontages is empty!"), *GetName());
		return false;
	}

	TArray<int32> ValidIndices;
	for (int32 i = 0; i < IdleMontages.Num(); ++i)
	{
		if (IdleMontages[i] != nullptr)
		{
			ValidIndices.Add(i);
		}
	}

	if (ValidIndices.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Idle] %s: All IdleMontages slots are null!"), *GetName());
		return false;
	}

	const int32 PickedIndex = ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)];
	UE_LOG(LogTemp, Log, TEXT("[Idle] %s: Playing IdleMontage[%d] (valid count=%d)"),
		*GetName(), PickedIndex, ValidIndices.Num());

	Multicast_PlayIdleMontage(PickedIndex);
	return true;
}

void AAIMonsterBase::Multicast_PlayIdleMontage_Implementation(int32 MontageIndex)
{
	if (!IdleMontages.IsValidIndex(MontageIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Idle] %s: Invalid montage index %d"), *GetName(), MontageIndex);
		return;
	}

	UAnimMontage* Montage = IdleMontages[MontageIndex];
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Idle] %s: Montage at index %d is null"), *GetName(), MontageIndex);
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Idle] %s: AnimInstance is null"), *GetName());
		return;
	}

	AnimInstance->Montage_Play(Montage);
	UE_LOG(LogTemp, Log, TEXT("[Idle] %s: Montage_Play called — %s"), *GetName(), *Montage->GetName());

	// 서버에서만 종료 콜백 바인딩
	if (HasAuthority())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAIMonsterBase::OnIdleMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void AAIMonsterBase::OnIdleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Log, TEXT("[Idle] %s: Montage ended (interrupted=%d)"), *GetName(), bInterrupted);
	OnIdleMontageFinished.ExecuteIfBound();
}

void AAIMonsterBase::ReceiveDamage(float Damage)
{
	if (!HasAuthority()) return;
	if (bIsDying)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReceiveDamage] %s: Already dying, ignoring damage"), *GetName());
		return;
	}

	// 공격자 정보가 없으면 자동으로 가장 가까운 플레이어 찾기
	if (!LastAttackerController)
	{
		float ClosestDistance = 1000.0f;  // 10m 이내
		
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;

			float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				LastAttackerController = PC;
				ClosestDistance = Distance;
				UE_LOG(LogTemp, Warning, TEXT("[Damage] Auto-detected attacker: %s (%.1fm away)"), 
					*PC->GetName(), Distance / 100.0f);
			}
		}
	}

	if (StatComponent)
	{
		float BeforeHp = StatComponent->GetHP();
		StatComponent->TakeDamage(Damage);

		UE_LOG(LogTemp, Warning,
			TEXT("[Monster Hit] %s | Dmg: %.1f | HP: %.1f -> %.1f"),
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

void AAIMonsterBase::ReceiveDamageFrom(float Damage, AController* InstigatorController)
{
	if (!HasAuthority()) return;
	if (bIsDying) return;

	// 마지막 공격자 저장
	if (InstigatorController && InstigatorController->IsA(APlayerController::StaticClass()))
	{
		LastAttackerController = Cast<APlayerController>(InstigatorController);
		UE_LOG(LogTemp, Log, TEXT("[DamageFrom] %s: Last attacker set to %s"), 
			*GetName(), *LastAttackerController->GetName());
	}

	// 기존 ReceiveDamage 로직 실행
	ReceiveDamage(Damage);
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
	DOREPLIFETIME(AAIMonsterBase, LastAttackerController);  
}

void AAIMonsterBase::UpdateHPBarWidget()
{
	if ( !HPWidgetComponent ) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if ( !PC ) return;

	// ----- 카메라 위치 얻기 -----
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// ----- 위젯 위치 -----
	const FVector WidgetLocation = HPWidgetComponent->GetComponentLocation();

	// ----- 카메라를 바라보게 회전 -----
	FVector Direction = CameraLocation - WidgetLocation;
	FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();

	// ----- Pitch, Roll 제거 ------
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Roll = 0.f;

	HPWidgetComponent->SetWorldRotation(LookAtRotation);

	// --- 화면상 HP UI 크기 유지용 스케일 ---
	const FVector DesiredScale(0.5f, 0.5f, 0.5f);
	HPWidgetComponent->SetWorldScale3D(DesiredScale);
}