#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UK_GolemMonster.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS()
class UK_API UUK_GolemMonster : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UUK_GolemMonster();
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeInitializeAnimation() override;
	
private:
	ACharacter* Owner;
	UCharacterMovementComponent* Movement;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector Velocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	float MovementSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	uint8 bIsfalling : 1;
	
};
