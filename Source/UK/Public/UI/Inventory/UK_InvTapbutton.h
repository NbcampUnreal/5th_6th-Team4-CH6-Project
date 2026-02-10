#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_InvTapbutton.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnInvTapButtonClicked, UUK_InvTapbutton*, OnButtonTap
);

UCLASS()
class UK_API UUK_InvTapbutton : public UUserWidget
{
	GENERATED_BODY()

public:

	// 바인드
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ButtonText;

	UPROPERTY(meta = ( BindWidget ))
	UButton* ButtonTap;
	
	// WBP Text 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvButtonTap")
	FText InText;


	//델리게이트 속성, 이벤트 호출 시 실행될 동작
	UPROPERTY(BlueprintAssignable, Category = "InvMainTap")
	FOnInvTapButtonClicked OnButtonTap;

	// 디자인 런타임 및 게임 실행 런타임 수정
	virtual void NativePreConstruct() override;

	// 디자인 런타임 X 게임 실행 런타임 수정 
	virtual void NativeConstruct() override;

	UFUNCTION()
	void ButtonTapClicked();
};
