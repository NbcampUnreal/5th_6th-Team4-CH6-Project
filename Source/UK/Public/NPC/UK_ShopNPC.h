#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UK_ShopNPC.generated.h"

UCLASS()
class UK_API AUK_ShopNPC : public ACharacter
{
	GENERATED_BODY()

public:
	AUK_ShopNPC();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
