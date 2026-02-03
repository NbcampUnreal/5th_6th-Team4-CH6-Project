// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/StatusComponent.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UStatusComponent::UStatusComponent() 
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}



void UStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatusComponent, Status);
}

// Called when the game starts
void UStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	Status.CurrentHp = Status.MaxHp;
}
// Called every frame
//void UStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	// ...
//}

#pragma region Status

void UStatusComponent::SetHp(const float CurrentHp)
{
	if ( GetOwnerRole() != ROLE_Authority || !IsValid(GetOwner()) )
	{
		return;
	}

	Status.CurrentHp = FMath::Clamp(CurrentHp, 0.f, Status.MaxHp);

	HpStatusDelegate.Broadcast(Status.CurrentHp, Status.MaxHp);

	if ( IsDead() )
	{
		OnDeadDelegate.Broadcast();
	}
}

bool UStatusComponent::IsDead() const
{
	return Status.CurrentHp <= 0.f;
}

void UStatusComponent::OnRepStatus()
{
	HpStatusDelegate.Broadcast(Status.CurrentHp, Status.MaxHp);
}

#pragma endregion

#pragma region Battle

float UStatusComponent::ApplyDamage()
{
	return Status.Str;
}

void UStatusComponent::TakeDamage(const float Damage)
{
	if( !IsValid(GetOwner()) || GetOwnerRole() != ROLE_Authority )
		return;

	float FinalDamage = FMath::Max(1.f, Damage);

	SetHp(Status.CurrentHp - FinalDamage);

	UE_LOG(LogTemp, Warning, TEXT("Damage : %f Charactor HP : %f"), FinalDamage, Status.CurrentHp);
}
#pragma endregion
