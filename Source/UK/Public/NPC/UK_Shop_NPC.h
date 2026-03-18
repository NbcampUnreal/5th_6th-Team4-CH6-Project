#pragma once

#include "CoreMinimal.h"
#include "NPC/UK_NPCAIBase.h"
#include "Character/UK_CharacterBase.h"
#include "UK_Shop_NPC.generated.h"

UCLASS()
class UK_API AUK_Shop_NPC : public AUK_NPCAIBase
{
	GENERATED_BODY()
	
public:
	AUK_Shop_NPC();
	
	virtual void Interact(AActor* Interactor) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TSubclassOf<class UUserWidget> ShopWidetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	class UDataTable* ShopItemTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FString ShopName;
};
