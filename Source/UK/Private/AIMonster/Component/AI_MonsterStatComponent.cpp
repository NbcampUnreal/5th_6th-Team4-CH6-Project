#include "AIMonster/Component/AI_MonsterStatComponent.h"
#include "Net/UnrealNetwork.h"

UAI_MonsterStatComponent::UAI_MonsterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAI_MonsterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 체력을 최대 체력으로 초기화시키기

	if ( GetOwnerRole() == ROLE_Authority )
	{
		Stats.CurrentHP = Stats.MaxHP;
	}
}

void UAI_MonsterStatComponent::OnRep_Stats()
{
	// 서버에서 변경된 HP를 클라이언트 UI에 전달용으로 쓰기

	OnHPChanged.Broadcast(Stats.CurrentHP, Stats.MaxHP);
}

bool UAI_MonsterStatComponent::IsDead() const
{
	return Stats.CurrentHP <= 0.f;
}

void UAI_MonsterStatComponent::TakeDamage(float Damage)
{
	//서버에서가 아니면 데미지 처리를 하지 않고 이미 죽었으면 데미지를 무시하게 설정

	if ( !GetOwner() || GetOwnerRole() != ROLE_Authority )
		return;

	if ( IsDead() )
		return;

	float FinalDamage = FMath::Max(1.f, Damage);

	SetHP(Stats.CurrentHP - FinalDamage);
}

void UAI_MonsterStatComponent::SetHP(float NewHP)
{
	if ( GetOwnerRole() != ROLE_Authority )
		return;

	// 체력을 0 ~ MaxHP 사이로 제한시키기
	Stats.CurrentHP = FMath::Clamp(NewHP, 0.f, Stats.MaxHP);

	OnHPChanged.Broadcast(Stats.CurrentHP, Stats.MaxHP);

	if ( IsDead() )
	{
		OnDeath.Broadcast();
	}
}

void UAI_MonsterStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAI_MonsterStatComponent, Stats);
}