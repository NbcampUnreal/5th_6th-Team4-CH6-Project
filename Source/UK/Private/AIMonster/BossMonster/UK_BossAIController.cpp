#include "AIMonster/BossMonster/UK_BossAIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AUK_BossAIController::AUK_BossAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}
void AUK_BossAIController::BeginPlay()
{
	Super::BeginPlay();
}
void AUK_BossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	BossPtr = Cast<AUK_BossMonsterBase>(InPawn);
	if (!BossPtr) return;

	if (BossPtr->BehaviorTree)
	{
		RunBehaviorTree(BossPtr->BehaviorTree);
	}
	
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;
	
	if (BB)
	{
		// 홈 위치 설정
		BB->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());

		// 초기 타겟 설정 (가장 가까운 플레이어로 확장 가능)
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player)
		{
			BB->SetValueAsObject(BB_TargetActor, Player);
		}

		// 2. [중요] 페이즈 동기화: 타이머 대신 보스의 델리게이트에 바인딩!
		// 보스 클래스에 OnBossPhaseChanged 델리게이트가 정의되어 있어야 합니다.
		BossPtr->OnBossPhaseChanged.AddDynamic(this, &AUK_BossAIController::HandlePhaseChanged);

		// 초기 페이즈 값 강제 동기화
		HandlePhaseChanged(BossPtr->CurrentPhaseTag);
	}
}

void AUK_BossAIController::OnUnPossess()
{
	if (BossPtr)
	{
		// 바인딩 해제
		BossPtr->OnBossPhaseChanged.RemoveAll(this);
	}

	BossPtr = nullptr;
	Super::OnUnPossess();
}

void AUK_BossAIController::HandlePhaseChanged(const FGameplayTag& NewPhaseTag)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB && NewPhaseTag.IsValid())
	{
		BB->SetValueAsName(BB_BossPhase, NewPhaseTag.GetTagName());
		UE_LOG(LogTemp, Warning, TEXT("AI Controller: Blackboard Updated Phase to %s"), *NewPhaseTag.ToString());
	}
}