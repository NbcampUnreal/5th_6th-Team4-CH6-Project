#pragma once

#include "CoreMinimal.h"
#include "AIMonster/Monster/UK_NormalMonster.h"
#include "UK_BurrowMonster.generated.h"

UCLASS()
class UK_API AUK_BurrowMonster : public AUK_NormalMonster
{
	GENERATED_BODY()

#pragma region Initialization
public:
	AUK_BurrowMonster();

protected:
	virtual void BeginPlay() override;
#pragma endregion

#pragma region Burrow
public:
	// 땅속에서 나오는 몽타주 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Burrow")
	TObjectPtr<UAnimMontage> EmergeMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Burrow")
	TObjectPtr<UAnimMontage> GetDownMontage;

	// AnimBP가 읽는 상태 플래그
	UPROPERTY(BlueprintReadOnly, Category="Burrow")
	bool bIsBurrowed = false;

	void SetBurrowed(bool bBurrow);

	// EmergeFromGround 태스크가 몽타주 완료 시 호출할 델리게이트
	FSimpleDelegate OnEmergeFinished;
#pragma endregion
};