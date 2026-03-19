#include "Systems/Sound/UK_SoundManager.h"
#include "Kismet/GameplayStatics.h"

AUK_SoundManager::AUK_SoundManager()
{
	PrimaryActorTick.bCanEverTick = false;
	BGMComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));
	BGMComponent->bAutoActivate = false;
}

void AUK_SoundManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUK_SoundManager::SetCurrentRegion(EBKRegion NewRegion, USoundBase* NewBGM)
{
	if (CurrentRegion == NewRegion && !bIsInCombat) return;

	CurrentRegion = NewRegion;
	
	// 전투 중이 아닐 때만 실제 사운드 교체
	if (!bIsInCombat)
	{
		PlayBGM(NewBGM);
	}
	else
	{
		// 전투 중이라면 복귀용 BGM 정보만 업데이트
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

		// 2. 현재 지역에 맞는 전투 BGM 찾기
		if (USoundBase** CombatBGM = CombatBGMMappings.Find(CurrentRegion))
		{
			PlayBGM(*CombatBGM);
		}
	}
	else
	{
		// 3. 전투 종료 시 저장해둔 필드 BGM으로 복귀
		if (LastFieldBGM)
		{
			PlayBGM(LastFieldBGM);
		}
	}
}

void AUK_SoundManager::PlayBGM(USoundBase* NewSound, bool bFade)
{
	if (!NewSound) return;

	if (bFade)
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
		
		// 약간의 딜레이 후 새 사운드 재생 (또는 타이머 사용 가능)
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewSound]()
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