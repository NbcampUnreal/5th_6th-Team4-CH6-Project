#include "Systems/Sound/UK_SoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AUK_SoundManager::AUK_SoundManager()
{
	PrimaryActorTick.bCanEverTick = false;
	BGMComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));
	BGMComponent->bAutoActivate = false;
}

// 스태틱 함수 구현
AUK_SoundManager* AUK_SoundManager::Get(const UObject* WorldContextObject)
{
	return Cast<AUK_SoundManager>(UGameplayStatics::GetActorOfClass(WorldContextObject, AUK_SoundManager::StaticClass()));
}

void AUK_SoundManager::BeginPlay()
{
	Super::BeginPlay();
}

void AUK_SoundManager::SetCurrentRegion(EBKRegion NewRegion, USoundBase* NewBGM)
{
	CurrentRegion = NewRegion;
	
	if (!bIsInCombat && BGMComponent->GetSound() == NewBGM) return;
	
	if (!bIsInCombat)
	{
		PlayBGM(NewBGM);
	}
	else
	{
		// 전투 중이면 나중에 돌아올 음악만 바꿔둠
		LastFieldBGM = NewBGM;
	}
}

void AUK_SoundManager::SetCombatState(bool bInCombat)
{
	if (this->bIsInCombat == bInCombat) return;
	this->bIsInCombat = bInCombat;

	if (bInCombat)
	{
		// 1. 현재 필드 BGM 저장
		LastFieldBGM = BGMComponent->Sound;

		// 현재 속해있는 상위지역 의 전투 음악 찾기
		if (USoundBase** CombatBGM = CombatBGMMappings.Find(CurrentRegion))
		{
			PlayBGM(*CombatBGM);
		}
	}
	else
	{
		// 3. 전투 종료 시 복귀
		if (LastFieldBGM)
		{
			PlayBGM(LastFieldBGM);
		}
	}
}

void AUK_SoundManager::PlayBGM(USoundBase* NewSound, bool bFade)
{
	if (!NewSound || BGMComponent->Sound == NewSound) return;

	if (bFade)
	{
		// 페이드 아웃 후 교체
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
		
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this, NewSound]()
		{
			BGMComponent->SetSound(NewSound);
			BGMComponent->FadeIn(FadeInDuration);
		}, FadeOutDuration, false);
	}
	else
	{
		BGMComponent->SetSound(NewSound);
		BGMComponent->Play();
	}
}