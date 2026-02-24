// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_Quest.h"

UUK_Quest::UUK_Quest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UUK_Quest::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Button_0 )
	{
		//Button_0->OnClicked.AddDynamic(this, &ThisClass::);
	}
	if ( Button_1 )
	{
		//Button_1->OnClicked.AddDynamic(this, &ThisClass::);
	}
}

void UUK_Quest::OnPlayButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Play Button Clicked"));
}

void UUK_Quest::OnExitButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Exit Button Clicked"));
}