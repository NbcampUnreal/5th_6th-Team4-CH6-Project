#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "UK_SoundManager.generated.h"

class AAmbientSound;

UENUM(BlueprintType)
enum class EBKRegion : uint8
{
	None,
	StartingVillage,
	Desert,
	Steampunk
};

UCLASS()
class UK_API AUK_SoundManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AUK_SoundManager();

	// 어디서든 사운드 매니저를 찾을 수 있게 해주는 도우미 함수
	UFUNCTION(blueprintCallable, BlueprintPure, Category = "SoundManager")
	static AUK_SoundManager* Get(const UObject* WorldContextObject);

protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sound")
	UAudioComponent* BGMComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sound")
	AAmbientSound* CurrentSound_cpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Settings")
	float FadeOutDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Settings")
	float FadeInDuration = 2.0f;

	// 지역별 전투 BGM (에디터에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Combat")
	TMap<EBKRegion, USoundBase*> CombatBGMMappings;

	// 기본 전투 BGM (해당 지역 전투음악이 없을 때 대비)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Combat")
	USoundBase* DefaultCombatBGM;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Combat")
	bool bIsInCombat = false;

private:
	EBKRegion CurrentRegion = EBKRegion::None;
	
	UPROPERTY()
	USoundBase* LastFieldBGM; // 전투 종료 후 복귀용

public:
	// 지역/구역 진입 시 호출
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void SetCurrentRegion(EBKRegion NewRegion);

	// 전투 상태 변경 시 호출
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void SetCombatState(bool bInCombat);

	void PlayBGM(USoundBase* NewSound, bool bFade = true);
};