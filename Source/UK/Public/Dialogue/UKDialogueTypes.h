#pragma once

#include "CoreMinimal.h"
#include "UKDialogueTypes.generated.h"

USTRUCT(BlueprintType)
struct FUKDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString next;
};

USTRUCT(BlueprintType)
struct FUKDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString speaker;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> emit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> setFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FUKDialogueChoice> choices;
};

USTRUCT(BlueprintType)
struct FUKDialogueDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString dialogueId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString startNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> requiresFlags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FUKDialogueNode> nodes;
};

USTRUCT(BlueprintType)
struct FUKDialogueRoot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FUKDialogueDef> dialogues;
};