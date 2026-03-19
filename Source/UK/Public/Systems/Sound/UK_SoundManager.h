#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "UK_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EBKRegion : uint8
{
	None,
	StartingVillage,
	Desert,
};

UCLASS()
class UK_API AUK_SoundManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AUK_SoundManager();

protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sound")
	UAudioComponent* BGMComponent; // 현재 재생 중인 컴포넌트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Settings")
	float FadeOutDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Settings")
	float FadeInDuration = 2.0f;

	// 지역별 전투 BGM을 저장할 맵 (에디터에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Combat")
	TMap<EBKRegion, USoundBase*> CombatBGMMappings;

private:
	EBKRegion CurrentRegion = EBKRegion::None;
	
	UPROPERTY()
	USoundBase* LastFieldBGM; // 전투 종료 후 복귀용 저장 변수

	bool bIsInCombat = false;

public:
	// 지역 진입 시 호출 (트리거 박스에서 호출용)
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void SetCurrentRegion(EBKRegion NewRegion, USoundBase* NewBGM);

	// 전투 상태 전환 (C++ 로직에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void SetCombatState(bool bInCombat);

	void PlayBGM(USoundBase* NewSound, bool bFade = true);
};
