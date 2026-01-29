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

/* 서버로 상태 요청을 보내는 구간 */

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

/* AI 활성화 제어 스위치 같은 역할 */

void AAIMonsterBase::SetAIActive(bool bActive)
{
   /* 로직 수정 중*/
}

/* 상태별 기본적인 동작 혹은 행위 */
/* 자식 클래스에서 상속 받아서 사용 될 함수 */

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
