// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UK_PlayerController_Title.generated.h"

/**
 * 
 */
UCLASS()
class UK_API AUK_PlayerController_Title : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void StartGame_mult(FString Text);

	UFUNCTION(BlueprintCallable)
	void StartGame();

	// 나중에 멀티용 서버
	UFUNCTION(Server, Reliable)
	void Server_StartGame();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = ( AllowPrivateAccess ))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = ( AllowPrivateAccess ))
	TObjectPtr<UUserWidget> UIWidgetInstance;
};
