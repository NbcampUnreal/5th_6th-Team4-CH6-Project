// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_AttackActionSoundNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"

void UUK_AttackActionSoundNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(MeshComp->GetOwner());
	if (IsValid(Player) == false)
		return;

	UGameplayStatics::SpawnSoundAttached(
		AttackSound,
		Player->GetMesh(),
		NAME_None,
		FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset,
		true,
		1.f,
		1.f,
		0.f,
		Player->Attenuation
	);
}
