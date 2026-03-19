#include "Dialogue/UKDialogueSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "Quest/UKQuestManagerSubsystem.h"
#include "DataAsset/NPCData/UK_NPCData.h"

void UUKDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUKDialogueSubsystem::Deinitialize()
{
	CurrentPackFileName.Empty();
	LoadedPack.Dialogues.Empty();
	bPackLoaded = false;
	CurrentDialogueId = NAME_None;
	CurrentNodeId = NAME_None;

	Super::Deinitialize();
}


// [2] Load / Start / End

bool UUKDialogueSubsystem::LoadDialoguePack(const FString& PackFileName)
{
	if ( PackFileName.IsEmpty() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] LoadDialoguePack failed: PackFileName is empty."));
		return false;
	}

	const FString AbsolutePath = BuildDialogueJsonAbsolutePath(PackFileName);

	FString JsonString;
	if ( !FFileHelper::LoadFileToString(JsonString, *AbsolutePath) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] Failed to load JSON file: %s"), *AbsolutePath);
		return false;
	}

	FUKDialoguePack ParsedPack;
	if ( !ParseDialoguePackFromJson(JsonString, ParsedPack) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] Failed to parse JSON file: %s"), *AbsolutePath);
		return false;
	}

	LoadedPack = ParsedPack;
	CurrentPackFileName = PackFileName;
	bPackLoaded = true;

	UE_LOG(LogTemp, Log, TEXT("[Dialogue] Pack loaded OK: %s  Dialogues=%d"),
		*PackFileName, LoadedPack.Dialogues.Num());

	return true;
}

bool UUKDialogueSubsystem::StartDialogue(const FString& PackFileName, FName DialogueId)
{
	if ( !bPackLoaded || CurrentPackFileName != PackFileName )
	{
		if ( !LoadDialoguePack(PackFileName) )
		{
			return false;
		}
	}

	if ( !CanStartDialogue(DialogueId) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] Cannot start dialogue. DialogueId=%s"), *DialogueId.ToString());
		return false;
	}

	const FUKDialogueData* Dialogue = FindDialogueById(DialogueId);
	if ( !Dialogue )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] DialogueId not found: %s"), *DialogueId.ToString());
		return false;
	}

	CurrentDialogueId = DialogueId;
	CurrentNodeId = Dialogue->StartNode;

	UE_LOG(LogTemp, Log, TEXT("[Dialogue] StartDialogue OK. DialogueId=%s StartNode=%s"),
		*CurrentDialogueId.ToString(), *CurrentNodeId.ToString());

	return true;
}

void UUKDialogueSubsystem::EndDialogue()
{
	CurrentDialogueId = NAME_None;
	CurrentNodeId = NAME_None;
}

bool UUKDialogueSubsystem::CanStartDialogue(FName DialogueId) const
{
	if ( !bPackLoaded ) return false;

	const FUKDialogueData* Dialogue = FindDialogueById(DialogueId);
	if ( !Dialogue ) return false;

	for ( const FName& RequiredFlag : Dialogue->RequiresFlags )
	{
		if ( !HasFlagKey(RequiredFlag) )
		{
			return false;
		}
	}

	return true;
}


// [3] Current State Getter

FName UUKDialogueSubsystem::GetCurrentSpeakerID() const
{
	const FUKDialogueNode* Node = GetCurrentNode();
	if ( !Node ) return NAME_None;

	return Node->Speaker;
}

FText UUKDialogueSubsystem::GetCurrentSpeakerName() const
{
	const FName SpeakerId = GetCurrentSpeakerID();
	if ( SpeakerId.IsNone() )
	{
		return FText::GetEmpty();
	}

	UGameInstance* GI = GetGameInstance();
	if ( !GI )
	{
		return FText::FromName(SpeakerId);
	}

	UUKQuestManagerSubsystem* QuestSubsystem = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSubsystem )
	{
		return FText::FromName(SpeakerId);
	}

	const FUK_NPCData* NPCRow = QuestSubsystem->GetNPCDataByNPCID(SpeakerId);
	if ( !NPCRow )
	{
		return FText::FromName(SpeakerId);
	}

	return NPCRow->NPCName;
}

FText UUKDialogueSubsystem::GetCurrentDialogueText() const
{
	const FUKDialogueNode* Node = GetCurrentNode();
	if ( !Node ) return FText::GetEmpty();

	return Node->Text;
}

TArray<FText> UUKDialogueSubsystem::GetCurrentChoiceTexts() const
{
	TArray<FText> Result;

	const FUKDialogueNode* Node = GetCurrentNode();
	if ( !Node ) return Result;

	for ( const FUKDialogueChoice& Choice : Node->Choices )
	{
		Result.Add(Choice.Text);
	}

	return Result;
}

int32 UUKDialogueSubsystem::GetCurrentChoiceCount() const
{
	const FUKDialogueNode* Node = GetCurrentNode();
	if ( !Node ) return 0;

	return Node->Choices.Num();
}

bool UUKDialogueSubsystem::IsDialogueEnd() const
{
	const FUKDialogueNode* Node = GetCurrentNode();
	if ( !Node ) return true;

	return Node->Choices.Num() == 0 || CurrentNodeId == FName(TEXT("N_END"));
}

FUKCurrentDialogueUIData UUKDialogueSubsystem::GetCurrentDialogueUIData() const
{
	FUKCurrentDialogueUIData OutData;

	OutData.DialogueId = CurrentDialogueId;
	OutData.NodeId = CurrentNodeId;
	OutData.SpeakerId = GetCurrentSpeakerID();
	OutData.SpeakerName = GetCurrentSpeakerName();
	OutData.DialogueText = GetCurrentDialogueText();
	OutData.bIsEnd = IsDialogueEnd();

	const TArray<FText> ChoiceTexts = GetCurrentChoiceTexts();
	for ( int32 i = 0; i < ChoiceTexts.Num(); ++i )
	{
		FUKDialogueChoiceUIData ChoiceUI;
		ChoiceUI.ChoiceIndex = i;
		ChoiceUI.ChoiceText = ChoiceTexts[ i ];
		OutData.Choices.Add(ChoiceUI);
	}

	return OutData;
}


// [4] Progress

bool UUKDialogueSubsystem::SelectChoice(int32 ChoiceIndex)
{
	const FUKDialogueNode* Node = GetCurrentNode();
	if ( !Node )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] SelectChoice failed: CurrentNode is null."));
		return false;
	}

	if ( !Node->Choices.IsValidIndex(ChoiceIndex) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] SelectChoice failed: invalid ChoiceIndex=%d"), ChoiceIndex);
		return false;
	}

	const FUKDialogueChoice& SelectedChoice = Node->Choices[ ChoiceIndex ];

	// 1) emit 처리 (퀘스트 시스템으로 이벤트 전달)
	ApplyEmitEvents(SelectedChoice.Emit);

	// 2) setFlags 처리
	// 현재 퀘스트 시스템에 "직접 SetFlag API"가 없으므로,
	// Accepted / Completed / Failed 같은 대표 플래그는 퀘스트 함수로 대체 처리
	ApplySetFlagsFallback(SelectedChoice.SetFlags);

	// 3) 다음 노드 이동
	if ( SelectedChoice.Next.IsNone() )
	{
		CurrentNodeId = FName(TEXT("N_END"));
	}
	else
	{
		CurrentNodeId = SelectedChoice.Next;
	}

	UE_LOG(LogTemp, Log, TEXT("[Dialogue] Choice selected. NextNode=%s"), *CurrentNodeId.ToString());

	return true;
}


// [6] Helpers

const FUKDialogueData* UUKDialogueSubsystem::FindDialogueById(FName DialogueId) const
{
	if ( !bPackLoaded ) return nullptr;

	for ( const FUKDialogueData& Dialogue : LoadedPack.Dialogues )
	{
		if ( Dialogue.DialogueId == DialogueId )
		{
			return &Dialogue;
		}
	}
	return nullptr;
}

const FUKDialogueNode* UUKDialogueSubsystem::FindNodeById(const FUKDialogueData& Dialogue, FName NodeId) const
{
	for ( const FUKDialogueNode& Node : Dialogue.Nodes )
	{
		if ( Node.Id == NodeId )
		{
			return &Node;
		}
	}
	return nullptr;
}

const FUKDialogueNode* UUKDialogueSubsystem::GetCurrentNode() const
{
	const FUKDialogueData* Dialogue = GetCurrentDialogue();
	if ( !Dialogue ) return nullptr;

	return FindNodeById(*Dialogue, CurrentNodeId);
}

const FUKDialogueData* UUKDialogueSubsystem::GetCurrentDialogue() const
{
	if ( CurrentDialogueId.IsNone() ) return nullptr;

	return FindDialogueById(CurrentDialogueId);
}

FString UUKDialogueSubsystem::BuildDialogueJsonAbsolutePath(const FString& PackFileName) const
{
	// 1) 이미 "Start/파일명.json"처럼 오면 그대로 사용
	if ( PackFileName.Contains(TEXT("/")) || PackFileName.Contains(TEXT("\\")) )
	{
		return FPaths::ProjectContentDir() / TEXT("Dialogue") / PackFileName;
	}

	// 2) 파일명만 오면 "D.Start.M.001-005.json"에서 Zone 추출
	TArray<FString> Parts;
	PackFileName.ParseIntoArray(Parts, TEXT("."), true);

	// 기대 형식: D.Start.M.001-005.json
	if ( Parts.Num() >= 2 )
	{
		const FString ZoneName = Parts[ 1 ];
		return FPaths::ProjectContentDir() / TEXT("Dialogue") / ZoneName / PackFileName;
	}

	// 3) 마지막 fallback
	return FPaths::ProjectContentDir() / TEXT("Dialogue") / PackFileName;
}

bool UUKDialogueSubsystem::ParseDialoguePackFromJson(const FString& JsonString, FUKDialoguePack& OutPack) const
{
	OutPack.Dialogues.Empty();

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if ( !FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid() )
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* DialoguesArray = nullptr;
	if ( !RootObject->TryGetArrayField(TEXT("dialogues"), DialoguesArray) || !DialoguesArray )
	{
		return false;
	}

	for ( const TSharedPtr<FJsonValue>& DialogueValue : *DialoguesArray )
	{
		if ( !DialogueValue.IsValid() ) continue;

		const TSharedPtr<FJsonObject> DialogueObj = DialogueValue->AsObject();
		if ( !DialogueObj.IsValid() ) continue;

		FUKDialogueData DialogueData;

		DialogueData.DialogueId = FName(*DialogueObj->GetStringField(TEXT("dialogueId")));
		DialogueData.StartNode = FName(*DialogueObj->GetStringField(TEXT("startNode")));

		// requiresFlags
		if ( DialogueObj->HasField(TEXT("requiresFlags")) )
		{
			const TArray<TSharedPtr<FJsonValue>>* RequiresFlagsArray = nullptr;
			if ( DialogueObj->TryGetArrayField(TEXT("requiresFlags"), RequiresFlagsArray) && RequiresFlagsArray )
			{
				for ( const TSharedPtr<FJsonValue>& FlagValue : *RequiresFlagsArray )
				{
					DialogueData.RequiresFlags.Add(FName(*FlagValue->AsString()));
				}
			}
		}

		// nodes
		const TArray<TSharedPtr<FJsonValue>>* NodesArray = nullptr;
		if ( DialogueObj->TryGetArrayField(TEXT("nodes"), NodesArray) && NodesArray )
		{
			for ( const TSharedPtr<FJsonValue>& NodeValue : *NodesArray )
			{
				if ( !NodeValue.IsValid() ) continue;

				const TSharedPtr<FJsonObject> NodeObj = NodeValue->AsObject();
				if ( !NodeObj.IsValid() ) continue;

				FUKDialogueNode NodeData;
				NodeData.Id = FName(*NodeObj->GetStringField(TEXT("id")));
				NodeData.Speaker = FName(*NodeObj->GetStringField(TEXT("speaker")));
				NodeData.Text = FText::FromString(NodeObj->GetStringField(TEXT("text")));

				// choices
				const TArray<TSharedPtr<FJsonValue>>* ChoicesArray = nullptr;
				if ( NodeObj->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray )
				{
					for ( const TSharedPtr<FJsonValue>& ChoiceValue : *ChoicesArray )
					{
						if ( !ChoiceValue.IsValid() ) continue;

						const TSharedPtr<FJsonObject> ChoiceObj = ChoiceValue->AsObject();
						if ( !ChoiceObj.IsValid() ) continue;

						FUKDialogueChoice ChoiceData;
						ChoiceData.Text = FText::FromString(ChoiceObj->GetStringField(TEXT("text")));

						if ( ChoiceObj->HasField(TEXT("next")) )
						{
							ChoiceData.Next = FName(*ChoiceObj->GetStringField(TEXT("next")));
						}

						// emit
						if ( ChoiceObj->HasField(TEXT("emit")) )
						{
							const TArray<TSharedPtr<FJsonValue>>* EmitArray = nullptr;
							if ( ChoiceObj->TryGetArrayField(TEXT("emit"), EmitArray) && EmitArray )
							{
								for ( const TSharedPtr<FJsonValue>& EmitValue : *EmitArray )
								{
									ChoiceData.Emit.Add(FName(*EmitValue->AsString()));
								}
							}
						}

						// setFlags
						if ( ChoiceObj->HasField(TEXT("setFlags")) )
						{
							const TArray<TSharedPtr<FJsonValue>>* SetFlagsArray = nullptr;
							if ( ChoiceObj->TryGetArrayField(TEXT("setFlags"), SetFlagsArray) && SetFlagsArray )
							{
								for ( const TSharedPtr<FJsonValue>& FlagValue : *SetFlagsArray )
								{
									ChoiceData.SetFlags.Add(FName(*FlagValue->AsString()));
								}
							}
						}

						NodeData.Choices.Add(ChoiceData);
					}
				}

				DialogueData.Nodes.Add(NodeData);
			}
		}

		OutPack.Dialogues.Add(DialogueData);
	}

	return true;
}

void UUKDialogueSubsystem::ApplyEmitEvents(const TArray<FName>& EventIds) const
{
	if ( EventIds.Num() == 0 ) return;

	UGameInstance* GI = GetGameInstance();
	if ( !GI ) return;

	UUKQuestManagerSubsystem* QuestSubsystem = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSubsystem ) return;

	for ( const FName& EventId : EventIds )
	{
		if ( EventId.IsNone() ) continue;

		UE_LOG(LogTemp, Log, TEXT("[Dialogue] Emit QuestEvent: %s"), *EventId.ToString());
		QuestSubsystem->EmitQuestEvent(EventId);
	}
}

void UUKDialogueSubsystem::ApplySetFlagsFallback(const TArray<FName>& FlagKeys) const
{
	if ( FlagKeys.Num() == 0 ) return;

	UGameInstance* GI = GetGameInstance();
	if ( !GI ) return;

	UUKQuestManagerSubsystem* QuestSubsystem = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSubsystem ) return;

	for ( const FName& FlagKey : FlagKeys )
	{
		if ( FlagKey.IsNone() ) continue;

		// 기대 형식: F.<QuestID>.<Category>
		TArray<FString> Parts;
		FlagKey.ToString().ParseIntoArray(Parts, TEXT("."), true);

		if ( Parts.Num() < 3 )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Dialogue] setFlags unsupported format: %s"), *FlagKey.ToString());
			continue;
		}

		const FString Prefix = Parts[ 0 ];
		const FString QuestIdStr = Parts[ 1 ];
		const FString Category = Parts[ 2 ];

		if ( Prefix != TEXT("F") )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Dialogue] setFlags non-F key not handled: %s"), *FlagKey.ToString());
			continue;
		}

		const FName QuestId(*QuestIdStr);

		// 현재 시스템에서 직접 가능한 최소 대응
		if ( Category == TEXT("Accepted") )
		{
			QuestSubsystem->StartQuest(QuestId);
		}
		else if ( Category == TEXT("Completed") )
		{
			QuestSubsystem->CompleteQuest(QuestId);
		}
		else
		{
			// 일반 Flag 세팅은 현재 QuestManager에 직접 API가 없으므로 로그만
			UE_LOG(LogTemp, Warning, TEXT("[Dialogue] setFlags fallback not implemented for: %s"), *FlagKey.ToString());
		}
	}
}

bool UUKDialogueSubsystem::HasFlagKey(FName FlagKey) const
{
	if ( FlagKey.IsNone() ) return false;

	UGameInstance* GI = GetGameInstance();
	if ( !GI ) return false;

	UUKQuestManagerSubsystem* QuestSubsystem = GI->GetSubsystem<UUKQuestManagerSubsystem>();
	if ( !QuestSubsystem ) return false;

	// 기대 형식: F.<QuestID>.<Category>
	TArray<FString> Parts;
	FlagKey.ToString().ParseIntoArray(Parts, TEXT("."), true);

	if ( Parts.Num() < 3 )
	{
		return false;
	}

	const FString Prefix = Parts[ 0 ];
	const FString QuestIdStr = Parts[ 1 ];
	// const FString Category = Parts[2];

	if ( Prefix != TEXT("F") )
	{
		return false;
	}

	// Common 전역 플래그는 아직 별도 저장소가 없어서 여기선 false 처리
	if ( QuestIdStr == TEXT("Common") )
	{
		return false;
	}

	FQuestProgress Progress;
	if ( !QuestSubsystem->GetProgress(FName(*QuestIdStr), Progress) )
	{
		return false;
	}

	return Progress.Flags.Contains(FlagKey);
}