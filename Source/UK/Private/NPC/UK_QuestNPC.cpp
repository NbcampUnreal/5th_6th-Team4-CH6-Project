#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/Component/UK_QuestComponent.h"
#include "Quest/UKQuestManagerSubsystem.h"

AUK_QuestNPC::AUK_QuestNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	bPlayerInRange = false;
	bReplicates = true;
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(1000.f); // 마커 띄울 범위 임시 설정

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerExit);

	QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
	QuestMarker->SetupAttachment(RootComponent);
	QuestMarker->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	QuestMarker->SetVisibility(false);

}

void AUK_QuestNPC::BeginPlay()
{
	Super::BeginPlay();

	bPlayerInRange = false;
}


void AUK_QuestNPC::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);

	if (Player && Player->InteractionComp)
	{
		bPlayerInRange = true;

		Player->InteractionComp->SetNearActor(this);

		if ( QuestMarker )
		{
			QuestMarker->SetVisibility(true);
			GetWorldTimerManager().SetTimer(
				MarkerTimerHandle,
				this,
				&AUK_QuestNPC::UpdateMarkerRotation,
				0.03f,
				true
			);
		}
	}
}

void AUK_QuestNPC::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);

	if (Player && Player->InteractionComp)

	{
		bPlayerInRange = false;

		Player->InteractionComp->ClearNearActor();

		if ( QuestMarker )
		{
			QuestMarker->SetVisibility(false);
			GetWorldTimerManager().ClearTimer(MarkerTimerHandle);
		}
	}
}

void AUK_QuestNPC::UpdateMarkerRotation()
{
	if (!QuestMarker) return;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	FVector ToPlayer = PlayerChar->GetActorLocation() - QuestMarker->GetComponentLocation();
	FRotator LookAtRotation = ToPlayer.Rotation();
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Yaw += 90.f;
	QuestMarker->SetWorldRotation(LookAtRotation);
}

void AUK_QuestNPC::Interact_Implementation(AActor* Interactor)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Interactor);
	if (!Player) return;

	if (HasAuthority())
	{
		Server_Interact(Player);
	}
	else
	{
		Server_Interact(Player);
	}
}

void AUK_QuestNPC::Server_Interact_Implementation(AUK_CharacterBase* Player)
{
	if ( !Player || !bPlayerInRange ) return;

	auto* QuestSys = GetGameInstance()->GetSubsystem<UUKQuestManagerSubsystem>();

	if (!QuestSys) return;

	const UUKQuestDefinitionAsset* Def = QuestSys->GetQuestDefinition(QuestID);

	if (!Def)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[QuestNPC] No Definition for %s"),
			*QuestID.ToString());
		return;
	}

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());

	if (!PlayerCtl) return;

	PlayerCtl->Client_ShowQuestUI(
		QuestID,
		Def->QuestTitle,       
		Def->NPCDialogue,      
		Def->QuestDescription  
	);
}




