#include "NPC/Component/UK_InteractionComponent.h"
#include "Net/UnrealNetwork.h"
#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Actor.h"

UUK_InteractionComponent::UUK_InteractionComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	NearActor = nullptr;
}

void UUK_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUK_InteractionComponent::SetNearActor(AActor* NewActor)
{
	NearActor = NewActor;
}

void UUK_InteractionComponent::ClearNearActor()
{
	NearActor = nullptr;
}

void UUK_InteractionComponent::TryInteract()
{
	if (!NearActor) return;

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_TryInteract(NearActor);
	}
	else
	{
		Server_TryInteract(NearActor);
	}
}

void UUK_InteractionComponent::Server_TryInteract_Implementation(AActor* Target)
{
	if ( !Target ) return;

	AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(Target);
	if ( !NPC ) return;

	NPC->Interact(GetOwner());
}
