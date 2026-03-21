#include "Systems/Sound/UK_SoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/AmbientSound.h"

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

void AUK_SoundManager::SetCurrentRegion(EBKRegion NewRegion)
{
	// 지역 정보 업데이트
	CurrentRegion = NewRegion;
}

void AUK_SoundManager::SetCombatState(bool bInCombat)
{
	if (this->bIsInCombat == bInCombat) return;
	this->bIsInCombat = bInCombat;

	if (bInCombat)
	{
		if (IsValid(CurrentSound_cpp))
		{
			CurrentSound_cpp->GetAudioComponent()->FadeOut(FadeOutDuration, 0.0f);
		}

		// 2. 현재 지역(CurrentRegion)에 맞는 전투 음악 찾아서 재생
		if (USoundBase** CombatBGM = CombatBGMMappings.Find(CurrentRegion))
		{
			UE_LOG(LogTemp, Display, TEXT("CombatBGMStart"))
			PlayBGM(*CombatBGM);
		}
	}
	else
	{
		if (IsValid(CurrentSound_cpp))
		{
			BGMComponent->FadeOut(FadeOutDuration, 0.0f);
			
			CurrentSound_cpp->GetAudioComponent()->Play();
		}
	}
}

void AUK_SoundManager::PlayBGM(USoundBase* NewSound, bool bFade)
{
	if (!NewSound) return;

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