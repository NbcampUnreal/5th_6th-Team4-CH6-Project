// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/UK_InputCheckNotify.h"
#include "Character/UK_CharacterBase.h"

void UUK_InputCheckNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp))
	{
		TObjectPtr< AUK_CharacterBase > AttackingCharacter = Cast<AUK_CharacterBase>(MeshComp->GetOwner());
		if (IsValid(AttackingCharacter))
		{
			AttackingCharacter->HandleOnCheckInputAttack();
		}
	}
}
