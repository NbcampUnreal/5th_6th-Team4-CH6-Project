// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/UK_CheckAttackNotify.h"
#include "Character/UK_CharacterBase.h"

void UUK_CheckAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp))
	{
		TObjectPtr< AUK_CharacterBase> Character = Cast<AUK_CharacterBase>(MeshComp->GetOwner());
		if (IsValid(Character))
		{
			Character->HandleOnCheckHit();
		}
	}
}
