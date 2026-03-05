#include "Animation/AnimNotify/UK_VFXAnimNotify.h"
#include "Character/UK_CharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UUK_VFXAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AUK_CharacterBase* OwnerCharacter = Cast<AUK_CharacterBase>(MeshComp->GetOwner());
	if (!IsValid(OwnerCharacter))
		return;
	
	if (OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy)
		return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (!ASC)
		return;

	FGameplayEventData EventData;
	EventData.EventTag = VFXEventTag;
	EventData.Instigator = OwnerCharacter;
	EventData.OptionalObject = Animation;

	ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
}