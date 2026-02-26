// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GA/UK_TargetActorTrace.h"
#include "Character/UK_CharacterBase.h"
#include "Abilities/GameplayAbility.h"
int32 AUK_TargetActorTrace::ShowAttackDebug = 1;

//static AUK_TargetActorTrace CVarShowAttackDebug(
//	TEXT("UK.ShowAttackDebug"),
//	AUK_TargetActorTrace::ShowAttackDebug,
//	TEXT(""),
//	ECVF_Cheat
//);
AUK_TargetActorTrace::AUK_TargetActorTrace() : 
	TraceStartSocketName("StartSocket"),
	TraceEndSocketName("EndSocket")
{
}

void AUK_TargetActorTrace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
}

void AUK_TargetActorTrace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());

	if ( IsConfirmTargetingAllowed() )
	{
		TArray<TWeakObjectPtr<AActor>> HitResult = GetTraceResult(SourceActor);
		FGameplayAbilityTargetData_ActorArray* ActorArray = new FGameplayAbilityTargetData_ActorArray();
		ActorArray->SetActors(HitResult);
		FGameplayAbilityTargetDataHandle DataHandle(ActorArray);
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
}

TArray<TWeakObjectPtr<AActor>> AUK_TargetActorTrace::GetTraceResult(AActor* InSourceActor)
{
	TArray<TWeakObjectPtr<AActor>> TargetActors;
	AUK_CharacterBase* OwnerCharactor = Cast<AUK_CharacterBase>(SourceActor);
	if ( IsValid(OwnerCharactor) == false )
	{
		UE_LOG(LogTemp, Display, TEXT("Chactor Cast Falied In AUK_TargetActorTrace::GetTraceResult"));
		return TargetActors;
	}
	if ( !IsValid(OwnerCharactor->GetRightHandWeapon()) )
		return TargetActors;

	FVector TraceStart = OwnerCharactor->GetRightHandWeapon()->GetSocketLocation(TraceStartSocketName);
	FVector TraceEnd = OwnerCharactor->GetRightHandWeapon()->GetSocketLocation(TraceEndSocketName);

	const float CapsuleRadius = 50.f;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerCharactor);
	CollisionParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> HitResult;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(CapsuleRadius);

	bool bIsHit = GetWorld()->SweepMultiByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_ATTACK,
		CollisionShape,
		CollisionParams
	);
	if ( ShowAttackDebug )
	{
#if ENABLE_DRAW_DEBUG
		FColor DrawColor = bIsHit ? FColor::Green : FColor::Red;

		FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat();
		DrawDebugCapsule(
			GetWorld(),
			( TraceStart + TraceEnd ) / 2,
			( TraceEnd - TraceStart ).Size(),
			CapsuleRadius,
			CapsuleRot,
			DrawColor,
			false,
			1.f
		);
#endif
	}

	for ( const FHitResult& Hit : HitResult )
	{
		if ( Hit.GetActor() )
		{
			TargetActors.Add(Hit.GetActor());
		}
	}

	return TargetActors;
}
