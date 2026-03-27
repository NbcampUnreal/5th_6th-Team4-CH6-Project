
#include "NPC/UK_NPCAICtl.h"
#include "NPC/UK_NPCAIBase.h"
#include "NPC/UK_PatrolNPC.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Systems/Time/UK_TimeSubsystem.h"
#include "Character/UK_CharacterBase.h"
#include "NPC/UK_NPCDataTypes.h"

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
		return;
	}

	if (!NPC->BehaviorTree)
	{
		return;
	}

	if (NPC->BehaviorTree->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*NPC->BehaviorTree->BlackboardAsset);
	}

	if (UWorld* World =GetWorld())
	{
		UUK_TimeSubsystem* TimeSys = World->GetSubsystem<UUK_TimeSubsystem>();
		if (TimeSys)
		{
			TimeSys->OnHourChanged.AddDynamic(this, &AUK_NPCAICtl::UpdateScheduleByTime);
			
			UpdateScheduleByTime(TimeSys->CurrentHour);
		}
	}
	
	BTComp->StartTree(*NPC->BehaviorTree);

}

void AUK_NPCAICtl::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if ( !BB ) return;

	AUK_CharacterBase* Player =Cast<AUK_CharacterBase>(Actor);
	AUK_PatrolNPC* NPC = Cast<AUK_PatrolNPC>(GetPawn());
	if ( !Player ) return;

	// 감지됨
	if ( Stimulus.WasSuccessfullySensed() )
	{
		BB->SetValueAsObject("Player", Player);
		BB->SetValueAsBool("IsAvoiding", true);
		NPC->bIsWaiting = false;
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

void AUK_NPCAICtl::UpdateScheduleByTime(int32 CurrentHour)
{
	if (!GetWorld() || HasAnyFlags(RF_ClassDefaultObject)) return;
	APawn* CurrentPawn = GetPawn();
	
	if (!CurrentPawn) 
	{
		return;
	}
	
	AUK_NPCAIBase* NPC = Cast<AUK_NPCAIBase>(CurrentPawn);
	if (!NPC || !NPC->ScheduleTable || !BlackboardComp) 
	{
		return;
	}
	
	TArray<FNPCScheduleRow*> AllRows;
	NPC->ScheduleTable->GetAllRows(TEXT(""), AllRows);
	
	for (auto* Row : AllRows)
	{
		if (Row->NPC_ID == NPC->MyNPC_ID && Row->StartHour == CurrentHour)
		{
			AActor* GoalActor = Row->TargetActor.LoadSynchronous();
			if (GoalActor)
			{
				FVector TargetPos = GoalActor->GetActorLocation();
				BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), TargetPos);
				BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)Row->ActivityState);
			}

			BlackboardComp->SetValueAsObject(TEXT("ActionMontage"), Row->ActionMontage);
			break;
		}
	}
}
