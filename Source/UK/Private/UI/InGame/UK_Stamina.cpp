#include "UI/InGame/UK_Stamina.h"
#include "Components/ProgressBar.h"
//#include "ActorComponent/StatusComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UUK_Stamina::NativeConstruct()
{
	Super::NativeConstruct();
}

// void UUK_Stamina::BindStatusComponent(UStatusComponent* NewStatusComp)
// {
// 	if ( !NewStatusComp ) return;
//
// 	if ( CachedStatusComp )
// 	{
// 		CachedStatusComp->StaminaStatusDelegate.RemoveAll(this);
// 	}
// 	
// 	CachedStatusComp = NewStatusComp;
// 	
// 	CachedStatusComp->StaminaStatusDelegate.AddDynamic(
// 		this,
// 		&UUK_Stamina::UpdateStaminaBar
// 	);
// 	
// 	// 초기값 반영
// 	FStructProperty* StatusProp = FindFieldChecked<FStructProperty>(UStatusComponent::StaticClass(), TEXT("Status"));
// 	if ( StatusProp )
// 	{
// 		const FStatus* StatusPtr = StatusProp->ContainerPtrToValuePtr<FStatus>(CachedStatusComp);
// 		if ( StatusPtr )
// 		{
// 			UpdateStaminaBar(StatusPtr->CurrentStamina, StatusPtr->MaxStamina);
// 		}
// 	}
// }

void UUK_Stamina::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
{
	if ( MaxStamina <= 0.f ) return;

	// 1. 비율 계산
	Percent = CurrentStamina / MaxStamina;

	// 2. 블루프린트 스크립트 실행 (머티리얼 인스턴스 업데이트 로직 트리거)
	// 헤더에 정의한 BlueprintImplementableEvent를 호출합니다.
	K2_OnUpdateStaminaPercent(Percent);
}

void UUK_Stamina::SetTrackingPosition(const FVector2D& ScreenPos, float Scale, bool bVisible)
{
	if ( !bVisible )
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);
	SetPositionInViewport(ScreenPos, true);
	SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	SetRenderScale(FVector2D(Scale, Scale));
}