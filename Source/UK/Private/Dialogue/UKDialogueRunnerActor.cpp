#include "Dialogue/UKDialogueRunnerActor.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

#include "Quest/UKQuestManagerSubsystem.h"

AUKDialogueRunnerActor::AUKDialogueRunnerActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUKDialogueRunnerActor::BeginPlay()
{
	Super::BeginPlay();

	if ( bAutoStartOnBeginPlay )
	{
		StartDialogueById(AutoStartDialogueId);
	}
}

bool AUKDialogueRunnerActor::LoadJsonIfNeeded()
{
	if ( bLoaded ) return true;

	const FString FullPath = FPaths::ProjectContentDir() / RelativeJsonPath;

	FString JsonStr;
	if ( !FFileHelper::LoadFileToString(JsonStr, *FullPath) )
	{
		UE_LOG(LogTemp, Error, TEXT("[DialogueRunner] Cannot read JSON: %s"), *FullPath);
		return false;
	}

	if ( !FJsonObjectConverter::JsonObjectStringToUStruct(JsonStr, &Root, 0, 0) )
	{
		UE_LOG(LogTemp, Error, TEXT("[DialogueRunner] JSON parse failed"));
		return false;
	}

	bLoaded = true;
	UE_LOG(LogTemp, Log, TEXT("[DialogueRunner] Loaded dialogues=%d"), Root.dialogues.Num());
	return true;
}

bool AUKDialogueRunnerActor::CheckRequiresFlags(const FUKDialogueDef& D) const
{
	for ( const FString& Req : D.requiresFlags )
	{
		if ( !HasFlag(FName(*Req)) )
			return false;
	}
	return true;
}

bool AUKDialogueRunnerActor::StartDialogueById(const FString& DialogueId)
{
	if ( !LoadJsonIfNeeded() ) return false;

	ActiveDialogue = nullptr;
	ActiveNode = nullptr;
	bRunning = false;

	for ( const FUKDialogueDef& D : Root.dialogues )
	{
		if ( D.dialogueId == DialogueId )
		{
			if ( !CheckRequiresFlags(D) )
			{
				UE_LOG(LogTemp, Warning, TEXT("[DialogueRunner] RequiresFlags not satisfied: %s"), *DialogueId);
				return false;
			}

			ActiveDialogue = &D;
			ActiveNode = FindNodeById(D.startNode);
			if ( !ActiveNode )
			{
				UE_LOG(LogTemp, Error, TEXT("[DialogueRunner] StartNode not found: %s"), *D.startNode);
				ActiveDialogue = nullptr;
				return false;
			}

			bRunning = true;

			ApplyNodeSideEffects(*ActiveNode);
			PrintCurrentNode();
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[DialogueRunner] DialogueId not found: %s"), *DialogueId);
	return false;
}

const FUKDialogueNode* AUKDialogueRunnerActor::FindNodeById(const FString& NodeId) const
{
	if ( !ActiveDialogue ) return nullptr;

	for ( const FUKDialogueNode& N : ActiveDialogue->nodes )
	{
		if ( N.id == NodeId )
			return &N;
	}
	return nullptr;
}

void AUKDialogueRunnerActor::ApplyNodeSideEffects(const FUKDialogueNode& Node)
{
	// setFlags
	for ( const FString& F : Node.setFlags )
	{
		SetFlag(FName(*F));
		UE_LOG(LogTemp, Log, TEXT("[DialogueRunner] setFlag: %s"), *F);
	}

	// emit (현재는 로그 + QuestManager.EmitQuestEvent 호출)
	if ( UGameInstance* GI = GetGameInstance() )
	{
		if ( UUKQuestManagerSubsystem* QM = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
		{
			for ( const FString& E : Node.emit )
			{
				QM->EmitQuestEvent(FName(*E));
				UE_LOG(LogTemp, Log, TEXT("[DialogueRunner] emit: %s"), *E);
			}
		}
	}
}

void AUKDialogueRunnerActor::PrintCurrentNode() const
{
	if ( !ActiveNode ) return;

	UE_LOG(LogTemp, Log, TEXT("===== Dialogue [%s] Node [%s] ====="),
		ActiveDialogue ? *ActiveDialogue->dialogueId : TEXT("None"),
		*ActiveNode->id);

	UE_LOG(LogTemp, Log, TEXT("[%s] %s"), *ActiveNode->speaker, *ActiveNode->text);

	for ( int32 i = 0; i < ActiveNode->choices.Num(); ++i )
	{
		const auto& C = ActiveNode->choices[ i ];
		UE_LOG(LogTemp, Log, TEXT("  (%d) %s -> %s"), i + 1, *C.text, *C.next);
	}
}

void AUKDialogueRunnerActor::ChooseIndex(int32 ChoiceIndex)
{
	if ( !bRunning || !ActiveNode )
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialogueRunner] Not running"));
		return;
	}

	const int32 Idx = ChoiceIndex - 1;
	if ( !ActiveNode->choices.IsValidIndex(Idx) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialogueRunner] Invalid choice index: %d"), ChoiceIndex);
		return;
	}

	const FString Next = ActiveNode->choices[ Idx ].next;

	if ( Next.Equals(TEXT("END"), ESearchCase::IgnoreCase) )
	{
		UE_LOG(LogTemp, Log, TEXT("[DialogueRunner] END"));
		bRunning = false;
		ActiveNode = nullptr;
		return;
	}

	const FUKDialogueNode* NextNode = FindNodeById(Next);
	if ( !NextNode )
	{
		UE_LOG(LogTemp, Error, TEXT("[DialogueRunner] Next node not found: %s"), *Next);
		return;
	}

	ActiveNode = NextNode;
	ApplyNodeSideEffects(*ActiveNode);
	PrintCurrentNode();
}

bool AUKDialogueRunnerActor::HasFlag(const FName Flag) const
{
	// 테스트 편의용: 공용 플래그 저장소
	const FName GlobalKey(TEXT("Q_Common_Flags"));

	if ( const UGameInstance* GI = GetGameInstance() )
	{
		if ( const UUKQuestManagerSubsystem* QM = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
		{
			const FQuestProgress* P = QM->RuntimeProgress.Find(GlobalKey);
			return P && P->Flags.Contains(Flag);
		}
	}
	return false;
}

void AUKDialogueRunnerActor::SetFlag(const FName Flag)
{
	const FName GlobalKey(TEXT("Q_Common_Flags"));

	if ( UGameInstance* GI = GetGameInstance() )
	{
		if ( UUKQuestManagerSubsystem* QM = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
		{
			FQuestProgress& P = QM->RuntimeProgress.FindOrAdd(GlobalKey);
			P.Flags.Add(Flag);
		}
	}
}