// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_WeaponBase.generated.h"

#pragma region Forward Declaration
class AUK_CharacterBase;
#pragma endregion

UCLASS()
class UK_API AUK_WeaponBase : public AActor
{
	GENERATED_BODY()
#pragma region Defualt

public:	
	// Sets default values for this actor's properties
	AUK_WeaponBase();

	// Called every frame
	//virtual void Tick(float DeltaTime) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform PlacementTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<AUK_CharacterBase> OwnerClass;
#pragma endregion
#pragma region GeterSeter
public:	
	AUK_CharacterBase* GetOwnerCharactor() const { return OwnerClass; };
	void SetOwnerCharactor(const TObjectPtr<AUK_CharacterBase> OwnerCharactor) { OwnerClass = OwnerCharactor; }
	
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; };

	FTransform GetWeaponTransform() const { return PlacementTransform; };
#pragma endregion

};
