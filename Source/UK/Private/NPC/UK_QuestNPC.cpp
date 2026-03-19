#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/Component/UK_QuestComponent.h"
#include "Quest/UKQuestManagerSubsystem.h"

#include "Blueprint/UserWidget.h"       
#include "GameFramework/PlayerController.h"

AUK_QuestNPC::AUK_QuestNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	bPlayerInRange = false;

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

	// NPCID는 BP 기본값 또는 배치된 액터에서 미리 지정되어 있어야 함
	if ( NPCID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] NPCID is None."));
		return;
	}

	UWorld* World = GetWorld();
	if ( !World )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] World is null."));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if ( !GI )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] GameInstance is null."));
		return;
	}

	UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSys )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] QuestManagerSubsystem is null."));
		return;
	}

	// NPCID로 서브시스템에서 NPC 데이터 조회
	const FUK_NPCData* Row = QuestSys->GetNPCDataByNPCID(NPCID);
	if ( !Row )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPC] NPC data not found for NPCID=%s"), *NPCID.ToString());
		return;
	}

	// DT에서 읽어온 값 복사
	NPCDisplayName = Row->NPCName;
	NPCDescription = Row->NPCDescription;

	UE_LOG(LogTemp, Log, TEXT("[NPC] Loaded from DT. NPCID=%s"), 
		*NPCID.ToString());
}


void AUK_QuestNPC::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void AUK_QuestNPC::Interact(AActor* Interactor)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Interactor);
	if ( !Player ) return;

	HandleQuestInteract(Player);
}

void AUK_QuestNPC::HandleQuestInteract(AUK_CharacterBase* Player)
{
	if (!Player || !bPlayerInRange) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;

	UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if (!QuestSys) return;

	AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());
	if (!PlayerCtl) return;

	const UUKQuestDefinitionAsset* Def = QuestSys->GetQuestDefinition(QuestID);

	FText QuestTitle = NPCDisplayName;
	FText Dialogue = NPCDescription;
	FText QuestDesc = FText::GetEmpty();

	if ( Def )
	{
		QuestTitle = Def->QuestTitle;
		Dialogue = Def->NPCDialogue;
		QuestDesc = Def->QuestDescription;
	}

	PlayerCtl->ShowQuestUI(
		QuestID,
		QuestTitle,
		Dialogue,
		QuestDesc
	);
}




