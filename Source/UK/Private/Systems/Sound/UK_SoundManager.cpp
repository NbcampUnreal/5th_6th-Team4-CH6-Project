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

void AUK_SoundManager::SetCurrentRegion(EBKRegion NewRegion, AAmbientSound* NewFieldSound)
{
	// 지역 정보 업데이트
	CurrentRegion = NewRegion;
	
	// 새로 들어온 사운드가 이미 재생중인 것과 다르면 무시
	if (CurrentSound_cpp == NewFieldSound) return;
	
	// 이전에 나오던 필드 소리가 있다면 페이드 아웃 후 정지
	if (IsValid(CurrentSound_cpp))
	{
		CurrentSound_cpp->GetAudioComponent()->FadeOut(FadeOutDuration, 0.0f);
	}
	
	// 정보 갱신
	CurrentRegion = NewRegion;
	CurrentSound_cpp = NewFieldSound;
	
	// 전투 중이 아닐 때만 새 필드 브금 페이드 인
	if (!bIsInCombat && IsValid(CurrentSound_cpp))
	{
		CurrentSound_cpp->GetAudioComponent()->FadeIn(FadeInDuration);
	}
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