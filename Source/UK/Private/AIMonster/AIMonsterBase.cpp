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
#include "Quest/UKQuestManagerSubsystem.h"

#pragma region Initialization
AAIMonsterBase::AAIMonsterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	StatComponent = CreateDefaultSubobject<UAI_MonsterStatComponent>(TEXT("StatComponent"));
	NetDormancy = DORM_Awake;

	bUseControllerRotationYaw = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement        = true;
		GetCharacterMovement()->bUseControllerDesiredRotation    = false;
		GetCharacterMovement()->RotationRate                     = FRotator(0.f, 540.f, 0.f);

		GetCharacterMovement()->NetworkSimulatedSmoothLocationTime             = 0.05f;
		GetCharacterMovement()->NetworkSimulatedSmoothRotationTime             = 0.15f;
		GetCharacterMovement()->ListenServerNetworkSimulatedSmoothLocationTime = 0.15f;
		GetCharacterMovement()->ListenServerNetworkSimulatedSmoothRotationTime = 0.15f;
		GetCharacterMovement()->NetworkMaxSmoothUpdateDistance                 = 92.f;
		GetCharacterMovement()->NetworkNoSmoothUpdateDistance                  = 140.f;
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

	NetUpdateFrequency    = 60.f;
	MinNetUpdateFrequency = 30.f;
}

void AAIMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = false;
		GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	}

	if (HasAuthority())
	{
		SpawnLocation = GetActorLocation();

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

	if (HasAuthority() && StatComponent)
	{
		StatComponent->OnDeath.RemoveAll(this);
		StatComponent->OnDeath.AddDynamic(this, &AAIMonsterBase::Die);
	}

	if (HPWidgetComponent && HPWidgetClass)
	{
		HPWidgetComponent->SetWidgetClass(HPWidgetClass);
		HPWidget = Cast<UUK_MonsterHealthBar>(HPWidgetComponent->GetUserWidgetObject());

		if (HPWidget && StatComponent)
		{
			HPWidget->BindMonsterStats(StatComponent);
		}
	}
	
	if (AlertWidgetComponent && AlertWidgetClass)
	{
		AlertWidgetComponent->SetWidgetClass(AlertWidgetClass);
	}
}
#pragma endregion

#pragma region State Management
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
		SetNetDormancy(DORM_DormantPartial);
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

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAIMonsterBase, CurrentState);
	DOREPLIFETIME(AAIMonsterBase, bIsAggressive);
	DOREPLIFETIME(AAIMonsterBase, Aggressor);
	DOREPLIFETIME(AAIMonsterBase, LastAttackerController);
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
	if (!HasAuthority()) return;
	PlayRandomAttackMontage();
}
#pragma endregion

#pragma region Combat
void AAIMonsterBase::ReceiveDamage(float Damage)
{
	if (!HasAuthority()) return;
	if (bIsDying) return;

	// 공격자 정보 없으면 가장 가까운 플레이어 자동 탐지
	if (!LastAttackerController)
	{
		float ClosestDistance = 1000.0f;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;

			const float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				LastAttackerController = PC;
				ClosestDistance        = Distance;
			}
		}
	}

	if (StatComponent)
	{
		StatComponent->TakeDamage(Damage);
	}

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

void AAIMonsterBase::ReceiveDamageFrom(float Damage, AController* InstigatorController)
{
	if (!HasAuthority()) return;
	if (bIsDying) return;

	if (InstigatorController && InstigatorController->IsA(APlayerController::StaticClass()))
	{
		LastAttackerController = Cast<APlayerController>(InstigatorController);
	}

	ReceiveDamage(Damage);
}
#pragma endregion

#pragma region Attack Animation
bool AAIMonsterBase::PlayRandomAttackMontage()
{
	if (bIsHit || bIsAttacking || bIsDying) return false;
	if (!HasAuthority()) return false;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return false;

	if (AttackMontages.Num() == 0) return false;

	const int32 RandomIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
	if (!AttackMontages[RandomIndex]) return false;

	bIsAttacking   = true;
	LastAttackTime = Now;

	Multicast_PlayAttackMontage(RandomIndex);
	return true;
}

void AAIMonsterBase::Multicast_PlayAttackMontage_Implementation(int32 MontageIndex)
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
	Multicast_PlayIdleMontage(PickedIndex);
	return true;
}

void AAIMonsterBase::Multicast_PlayIdleMontage_Implementation(int32 MontageIndex)
{
	if (!IdleMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = IdleMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(Montage);

	if (HasAuthority())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAIMonsterBase::OnIdleMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void AAIMonsterBase::OnIdleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnIdleMontageFinished.ExecuteIfBound();
}
#pragma endregion

#pragma region Hit Animation
bool AAIMonsterBase::PlayRandomHitMontage()
{
	if (bIsDying || !HasAuthority()) return false;
	if (HitMontages.Num() == 0) return false;

	TArray<int32> ValidIndices;
	for (int32 i = 0; i < HitMontages.Num(); ++i)
	{
		if (HitMontages[i]) ValidIndices.Add(i);
	}
	if (ValidIndices.Num() == 0) return false;

	const int32 PickedIndex = ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)];
	Multicast_PlayHitMontage(PickedIndex);
	return true;
}

void AAIMonsterBase::Multicast_PlayHitMontage_Implementation(int32 MontageIndex)
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
	if (!HasAuthority() || bIsDying) return;

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

	SetServerState(EMonsterState::Dead);

	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(
		DeathTimer, this,
		&AAIMonsterBase::FinalizeDeath,
		CorpseLingerTime > 0.f ? 1.0f : 0.1f,
		false);
}

void AAIMonsterBase::FinalizeDeath()
{
	if (!HasAuthority()) return;

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

void AAIMonsterBase::HideAndBroadcastDeath()
{
	if (!HasAuthority()) return;

	Multicast_HideCorpse();
	SetNetDormancy(DORM_DormantAll);

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
	if (!HasAuthority()) return;
	OnMonsterKilled.Broadcast(this, MonsterType, LastAttackerController);

	// 퀘스트 이벤트
	const FName EventId = GetMonsterEventId(MonsterType);
	if (EventId == NAME_None) return;

	if (UUKQuestManagerSubsystem* QM = GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>())
	{
		QM->EmitQuestEvent(EventId);
	}
}

void AAIMonsterBase::Multicast_HideCorpse_Implementation()
{
	SetActorHiddenInGame(true);
}
#pragma endregion

#pragma region Respawn
void AAIMonsterBase::ResetHealth()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(CorpseTimerHandle);

	// 플래그 리셋
	bIsAttacking           = false;
	bIsDying               = false;
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
		Aggressor     = nullptr;
		SetServerState(EMonsterState::Passive);
	}
	else
	{
		SetServerState(EMonsterState::Idle);
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

	// Dormancy 해제
	FlushNetDormancy();
	SetNetDormancy(DORM_Awake);

	// 외형 + 애니메이션 리셋
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
	if (!HasAuthority()) return;

	bIsAggressive = false;
	bIsAttacking  = false;
	Aggressor     = nullptr;

	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
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
	if (!HasAuthority()) return;
	Multicast_ShowAlertIcon();
}

void AAIMonsterBase::HideAlertIcon()
{
	if (!HasAuthority()) return;
	Multicast_HideAlertIcon();
}

void AAIMonsterBase::Multicast_ShowAlertIcon_Implementation()
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

void AAIMonsterBase::Multicast_HideAlertIcon_Implementation()
{
	if (!AlertWidgetComponent) return;

	if (UUserWidget* W = AlertWidgetComponent->GetUserWidgetObject())
		W->SetVisibility(ESlateVisibility::Collapsed);

	AlertWidgetComponent->SetVisibility(false);
}
#pragma endregion