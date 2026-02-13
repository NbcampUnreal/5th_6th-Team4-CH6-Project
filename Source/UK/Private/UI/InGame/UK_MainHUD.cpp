#include "UI/InGame/UK_MainHUD.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

void UUK_MainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 플레이어 캐릭터 가져오기
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if ( PlayerPawn )
	{
		// 캐릭터에서 StatusComponent 찾기
		UStatusComponent* StatusComp = PlayerPawn->FindComponentByClass<UStatusComponent>();
		if ( StatusComp )
		{
			// 3. 델리게이트 바인딩 
			StatusComp->HpStatusDelegate.AddDynamic(this, &UUK_MainHUD::UpdateHealthBar);

		}
	}
}

void UUK_MainHUD::UpdateHealthBar(float CurrentHp, float MaxHp)
{
	if ( HealthBar && MaxHp > 0.f )
	{
		// 0.0 ~ 1.0 사이의 퍼센트 값으로 변환하여 반영
		HealthBar->SetPercent(CurrentHp / MaxHp);
	}
}

