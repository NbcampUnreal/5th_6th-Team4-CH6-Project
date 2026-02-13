#include "Dialogue/UKDialogueTriggerActor.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

#include "Dialogue/UKDialogueRunnerActor.h"

AUKDialogueTriggerActor::AUKDialogueTriggerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AUKDialogueTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AUKDialogueTriggerActor::OnTriggerBeginOverlap);
}

void AUKDialogueTriggerActor::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if ( !OtherActor ) return;

	// 1회 발동 옵션
	if ( bTriggerOnce && bTriggered ) return;

	// 플레이어 Pawn인지 체크
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if ( OtherActor != PlayerPawn ) return;

	// 러너가 없으면 시작 불가
	if ( !RunnerRef )
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialogueTrigger] RunnerRef is null. Set RunnerRef in editor."));
		return;
	}

	// 러너가 이미 실행중이면 중복 시작 방지
	if ( RunnerRef->IsRunning() )
	{
		UE_LOG(LogTemp, Log, TEXT("[DialogueTrigger] Runner already running. Ignore."));
		return;
	}

	// 대화 시작
	if ( RunnerRef->StartDialogueById(DialogueIdToStart) )
	{
		bTriggered = true;
		UE_LOG(LogTemp, Log, TEXT("[DialogueTrigger] Started dialogue: %s"), *DialogueIdToStart);
	}
}