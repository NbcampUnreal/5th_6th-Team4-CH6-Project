#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"

AAIMonsterBase::AAIMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	NetDormancy = DORM_DormantAll;
}

void AAIMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetActorTickEnabled(false);
		SpawnLocation = GetActorLocation();
	}
}

void AAIMonsterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    switch (CurrentState)
    {
    case EMonsterState::Idle:
        OnIdle();
        break;

    case EMonsterState::Patrol:
        OnPatrol();
        break;

    case EMonsterState::Chase:
        OnChase(DeltaSeconds);
        break;

    case EMonsterState::Attack:
        OnAttack();
        break;

    case EMonsterState::Dead:
        OnDead();
        break;
    }
}

/* ������ ���� ��û�� ������ ���� */

void AAIMonsterBase::RequestState_Implementation(EMonsterState NewState)
{
    if (!HasAuthority()) return;
    if (CurrentState == NewState) return;

    SetServerState(NewState);
}

void AAIMonsterBase::SetServerState(EMonsterState NewState)
{
    CurrentState = NewState;

    if (CurrentState == EMonsterState::Dead)
    {
        DetachFromControllerPendingDestroy();
    }

    OnRep_MonsterState();
}

void AAIMonsterBase::OnRep_MonsterState()
{
    switch (CurrentState)
    {
    case EMonsterState::Idle:
        SetActorTickEnabled(false);
        SetNetDormancy(DORM_DormantAll);
        break;

    case EMonsterState::Patrol:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalPatrol;
        SetNetDormancy(DORM_Awake);
        break;

    case EMonsterState::Chase:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalChase;
        SetNetDormancy(DORM_Awake);
        break;

    case EMonsterState::Attack:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalAttack;
        SetNetDormancy(DORM_Awake);
        break;

    case EMonsterState::Dead:
        SetActorTickEnabled(false);
        SetNetDormancy(DORM_DormantAll);
        break;
    }
}

/* AI Ȱ��ȭ ���� ����ġ ���� ���� */

void AAIMonsterBase::SetAIActive(bool bActive)
{
   /* ���� ���� ��*/
}

/* Spawner System*/
void AAIMonsterBase::Die()
{
	if (!HasAuthority()) return;
	if (IsDead()) return;

	SetServerState(EMonsterState::Dead);
	OnDeath.Broadcast(this);
}

void AAIMonsterBase::ResetHealth()
{
	// HP 컴포넌트 추가시 여기서 초기화
	// 현재는 상태만 리셋
	if (HasAuthority())
	{
		SetServerState(EMonsterState::Idle);
	}
}

/* ���º� �⺻���� ���� Ȥ�� ���� */
/* �ڽ� Ŭ�������� ��� �޾Ƽ� ��� �� �Լ� */

void AAIMonsterBase::OnIdle()
{
}

void AAIMonsterBase::OnPatrol()
{
}

void AAIMonsterBase::OnChase(float DeltaSeconds)
{
}

void AAIMonsterBase::OnAttack()
{
}

void AAIMonsterBase::OnDead()
{
}

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAIMonsterBase, CurrentState);
}
