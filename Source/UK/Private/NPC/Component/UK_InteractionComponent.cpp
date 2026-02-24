#include "NPC/Component/UK_InteractionComponent.h"
#include "Net/UnrealNetwork.h"
#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Actor.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"

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
	if (!NewActor) return;
	NearActor = NewActor;

	UE_LOG(LogTemp, Log, TEXT("[Interaction] SetNearActor: %s"),
		*NewActor->GetName());
}

void UUK_InteractionComponent::ClearNearActor()
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());

	if (Player)
	{
		Player->Client_HideInteractUI();
		if ( PlayerCtl )
		{
			PlayerCtl->bShowMouseCursor = false;
			FInputModeGameOnly InputMode;
			PlayerCtl->SetInputMode(InputMode);
		}
	}
	NearActor = nullptr;
}

void UUK_InteractionComponent::TryInteract()
{
	if (!NearActor) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());

	if (Player)
	{
		Player->Client_ShowInteractUI();
		if (PlayerCtl)
		{
			PlayerCtl->bShowMouseCursor = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PlayerCtl->SetInputMode(InputMode);
		}
	}

	if (!Owner->HasAuthority())
	{
		Server_TryInteract(NearActor);
		return;
	}

	AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(NearActor);
	if (NPC)
	{
		NPC->Interact(Owner);
	}
}

void UUK_InteractionComponent::Server_TryInteract_Implementation(AActor* Target)
{
	if (!Target) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(Target);
	if (!NPC) return;

	NPC->Interact(GetOwner());
}
