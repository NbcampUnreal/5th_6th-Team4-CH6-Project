
#include "NPC/UK_NPCAICtl.h"
#include "NPC/UK_NPCAIBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UK_CharacterBase.h"

AUK_NPCAICtl::AUK_NPCAICtl()
{
	BlackboardComp =CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BTComp =CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComp"));

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 200.f;
	SightConfig->LoseSightRadius = 350.f;
	SightConfig->PeripheralVisionAngleDegrees = 30.f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AUK_NPCAICtl::BeginPlay()
{
	Super::BeginPlay();

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this,&AUK_NPCAICtl::OnPerceptionUpdated);
}

void AUK_NPCAICtl::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AUK_NPCAIBase* NPC = Cast<AUK_NPCAIBase>(InPawn);

	if (!NPC)
	{
		UE_LOG(LogTemp, Error, TEXT("NPC Cast Failed"));
		return;
	}

	if (!NPC->BehaviorTree)
	{
		UE_LOG(LogTemp, Error, TEXT("BehaviorTree is NULL"));
		return;
	}

	if (NPC->BehaviorTree->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*NPC->BehaviorTree->BlackboardAsset);
	}

	BTComp->StartTree(*NPC->BehaviorTree);

	UE_LOG(LogTemp, Warning, TEXT("BT Started: %s"), *GetName());
}

void AUK_NPCAICtl::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if ( !BB ) return;

	AUK_CharacterBase* Player =
		Cast<AUK_CharacterBase>(Actor);

	if ( !Player ) return;

	// 감지됨
	if ( Stimulus.WasSuccessfullySensed() )
	{
		BB->SetValueAsObject("Player", Player);
		BB->SetValueAsBool("IsAvoiding", true);
	}
	// 놓침
	else
	{
		BB->ClearValue("Player");
		BB->SetValueAsBool("IsAvoiding", false);
	}
}

void AUK_NPCAICtl::DrawSightDebug()
{
#if WITH_EDITOR
	if ( !GetPawn() ) return;

	FVector Start = GetPawn()->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	FVector Forward = GetPawn()->GetActorForwardVector();


	float SightRadius = SightConfig->SightRadius;
	float FOVAngle = SightConfig->PeripheralVisionAngleDegrees;

	DrawDebugCone(
		GetWorld(),
		Start,
		Forward,
		SightRadius,
		FMath::DegreesToRadians(FOVAngle / 2.f),
		FMath::DegreesToRadians(FOVAngle / 2.f),
		12,              
		FColor::Red,
		false,           
		0.1f,             
		0,                
		2.f              
	);

	DrawDebugSphere(
		GetWorld(),
		Start,
		SightRadius,
		24,
		FColor::Green,
		false,
		0.1f,
		0,
		2.f
	);
#endif
}