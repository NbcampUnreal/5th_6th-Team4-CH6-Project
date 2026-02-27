#include "NPC/Component/UK_InteractionComponent.h"
#include "Net/UnrealNetwork.h"
#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Actor.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"

UUK_InteractionComponent::UUK_InteractionComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	NearActor = nullptr;
}

void UUK_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUK_InteractionComponent::SetNearActor(AActor* NewActor)
{
	if (!NewActor) return;
	NearActor = NewActor;

	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	if (!Player) return;

}

void UUK_InteractionComponent::ClearNearActor()
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());

	if (Player)
	{
		AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());

		if (PlayerCtl)
		{
			PlayerCtl->Client_HideQuestUI();
		}
	}

	NearActor = nullptr;
}

void UUK_InteractionComponent::TryInteract()
{
	if (!NearActor) return;

	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	if (!Player) return;

	if (!Player->HasAuthority())
	{
		Server_TryInteract(NearActor);
		return;
	}

	AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(NearActor);
	if (NPC)
	{
		NPC->Interact(Player);
	}
}

void UUK_InteractionComponent::Server_TryInteract_Implementation(AActor* Target)
{
	if (!Target) return;

	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	if (!Player) return;

	AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(Target);
	if (!NPC) return;

	NPC->Interact(GetOwner());
}
