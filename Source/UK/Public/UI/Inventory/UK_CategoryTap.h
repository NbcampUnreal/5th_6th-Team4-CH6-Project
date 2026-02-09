#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_CategoryTap.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCategoryTapClicked, UUK_CategoryTap*, OnCategoryTap
);


UCLASS()
class UK_API UUK_CategoryTap : public UUserWidget
{
	GENERATED_BODY()
	

	
public:
	UPROPERTY(meta = ( BindWidget ))
	UButton* CategoryButton;

	//델리게이트 속성, 이벤트 호출 시 실행될 동작
	UPROPERTY(BlueprintAssignable, Category = "InvCategoryTap")
	FOnCategoryTapClicked OnCategoryTap;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void CategoryTapClicked();

};
