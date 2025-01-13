// Copyright Zachary Brett, 2024. All rights reserved.

//Header
#include "Dialogue.h"
//UE
#include "EdGraph/EdGraph.h"
#include "Kismet/GameplayStatics.h"
//Plugin
#include "DialogueController.h"
#include "DialogueSpeakerComponent.h"
#include "DialogueSpeakerSocket.h"
#include "LogDialogueTree.h"
#include "Nodes/DialogueEntryNode.h"

FColor FDefaultDialogueColors::PopColor()
{
	FColor TargetColor = Colors[ColorIndex];
	++ColorIndex;
	ColorIndex = ColorIndex % Colors.Num();

	return TargetColor;
}

UDialogue::UDialogue()
{
	//Add default speakers
	AddDefaultSpeakers();
}

#if WITH_EDITOR

void UDialogue::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	
	//If editing the speaker roles
	if (PropertyName.IsEqual("SpeakerRoles"))
	{
		OnChangeSpeakers(PropertyChangedEvent.ChangeType);
	}
}

#endif

// void UDialogue::SetSpeaker(FName InName, UDialogueSpeakerComponent* InSpeaker)
// {
// 	if (Speakers.Contains(InName))
// 	{
// 		Speakers[InName] = InSpeaker;
// 	}
// }
//
// UDialogueSpeakerComponent* UDialogue::GetSpeaker(FName InName) const
// {
// 	if (Speakers.Contains(InName))
// 	{
// 		return Speakers[InName];
// 	}
//
// 	return nullptr;
// }

void UDialogue::AddSpeakerEntry(FGameplayTag InTag)
{
	if (!Speakers.Contains(InTag))
	{
		Speakers.Add(InTag);
	}
}

void UDialogue::OpenDialogue(ADialogueController* InController, TArray<FGameplayTag> InSpeakerTags)
{
	//Make sure we can start the dialogue 
	FString ErrorMessage;
	if (!CanPlay(InController, ErrorMessage))
	{
		UE_LOG(
			LogDialogueTree,
			Error,
			TEXT("Cannot play dialogue. %s"),
			*ErrorMessage
		);
		return;
	}

	//Store the controller 
	DialogueController = InController;

	//Fill the speakers with the provided values 
	FillSpeakers(InSpeakerTags);

	//Traverse the first node 
	TraverseNode(RootNode);
}

void UDialogue::ClearController()
{
	DialogueController = nullptr;
}

void UDialogue::EndDialogue() const
{
	//End the dialogue
	if (DialogueController)
	{
		DialogueController->EndDialogue();
	}
}

void UDialogue::DisplaySpeech(const FSpeechDetails& InDetails) const
{
	if (!Speakers.Contains(InDetails.SpeakerTag))
	{
		EndDialogue();
		return;
	}

	DialogueController->DisplaySpeech(
		InDetails, 
		InDetails.SpeakerTag
	);
	DialogueController->OnDialogueSpeechDisplayed.Broadcast(InDetails);
}

void UDialogue::DisplayOptions(TArray<FDialogueOption> InOptions) const
{
	if (!DialogueController)
	{
		UE_LOG(
			LogDialogueTree,
			Error,
			TEXT("Attempting to display options via missing dialogue controller. Aborting dialogue.")
		);
		return;
	}

	TArray<FSpeechDetails> AllDetails;
	for (FDialogueOption Option : InOptions)
	{
		AllDetails.Add(Option.Details);
	}

	DialogueController->DisplayOptions(AllDetails);
}

void UDialogue::SelectOption(int32 InOptionIndex) const
{
	if (ActiveNode)
	{
		ActiveNode->SelectOption(InOptionIndex);
	}
}

void UDialogue::Skip() const
{
	if (ActiveNode)
	{
		ActiveNode->Skip();
	}
}

void UDialogue::TraverseNode(UDialogueNode* InNode)
{
	//return if the dialogue is already closed
	if (!DialogueController) 
	{
		return;
	}

	//If no node provided, end the dialogue
	if (!InNode) 
	{
		EndDialogue();
		return;
	}

	//Mark the node visited 
	DialogueController->MarkNodeVisited(this, InNode->GetNodeIndex());

	//Traverse the target node 
	ActiveNode = InNode;
	ActiveNode->EnterNode();
}

EDialogueCompileStatus UDialogue::GetCompileStatus() const
{
	return CompileStatus;
}

TArray<FGameplayTag> UDialogue::GetAllSpeakers() const
{
	return Speakers;
}

bool UDialogue::SpeakerIsPresent(const FGameplayTag SpeakerTag) const
{
	return Speakers.Find(SpeakerTag) != -1;
}

bool UDialogue::WasNodeVisited(UDialogueNode* TargetNode) const
{
	if (!DialogueController || !TargetNode 
		|| !DialogueNodes.Contains(TargetNode))
	{
		return false;
	}

	return DialogueController->WasNodeVisited(
		this, 
		TargetNode->GetNodeIndex()
	);
}

void UDialogue::MarkNodeVisited(UDialogueNode* TargetNode, bool bVisited)
{
	if (!DialogueController || !TargetNode)
	{
		return;
	}

	if (bVisited)
	{
		DialogueController->MarkNodeVisited(
			this, 
			TargetNode->GetNodeIndex()
		);
	}
	else
	{
		DialogueController->MarkNodeUnvisited(
			this,
			TargetNode->GetNodeIndex()
		);
	}
}

void UDialogue::ClearAllNodeVisits()
{
	if (!DialogueController)
	{
		return;
	}

	DialogueController->ClearAllNodeVisitsForDialogue(this);
}

#if WITH_EDITOR

UEdGraph* UDialogue::GetEdGraph() const
{
	return EdGraph;
}

void UDialogue::SetEdGraph(UEdGraph* InEdGraph)
{
	EdGraph = InEdGraph;
}

const TMap<FGameplayTag, FSpeakerField>& UDialogue::GetSpeakerRoles() const
{
	return SpeakerRoles;
}

void UDialogue::AddNode(UDialogueNode* InNode)
{
	if (InNode)
	{ 
		DialogueNodes.Add(InNode);
		InNode->SetDialogue(this);
	}
}

void UDialogue::RemoveNode(UDialogueNode* InNode)
{
	if (InNode)
	{
		DialogueNodes.Remove(InNode);
	}
}

void UDialogue::SetRootNode(UDialogueNode* InNode)
{
	if (UDialogueEntryNode* EntryNode 
		= Cast<UDialogueEntryNode>(InNode))
	{
		RootNode = EntryNode;
	}
}

UDialogueNode* UDialogue::GetRootNode() const
{
	return RootNode;
}

void UDialogue::ClearDialogue()
{
	RootNode = nullptr;
	DialogueNodes.Empty();
	Speakers.Empty();
	CompileStatus = EDialogueCompileStatus::Uncompiled;
}

void UDialogue::PreCompileDialogue()
{
	ClearDialogue();

	//Refresh the accessible speaker component entries from their roles
	for (auto& Entry : SpeakerRoles)
	{
		AddSpeakerEntry(Entry.Key);
	}
}

void UDialogue::PostCompileDialogue()
{
	//Notify nodes of their indices in the dialogue
	for (int32 NodeIndex = 0; NodeIndex < DialogueNodes.Num(); ++NodeIndex)
	{
		DialogueNodes[NodeIndex]->SetNodeIndex(NodeIndex);
	}
}

void UDialogue::SetCompileStatus(EDialogueCompileStatus InStatus)
{
	CompileStatus = InStatus;
	MarkPackageDirty(); //need to save
}
#endif

void UDialogue::AddDefaultSpeakers()
{
	//Default player
	// UDialogueSpeakerSocket* PlayerSpeaker =
	// 	CreateDefaultSubobject<UDialogueSpeakerSocket>(
	// 		"DefaultSpeakerPlayer"
	// 	);
	// KanatiNPCData::
	// PlayerSpeaker->SetSpeakerTag(NPCPlayer);
	// FSpeakerField PlayerField;
	// PlayerField.GraphColor = DefaultSpeakerColors.PopColor();
	// PlayerField.SpeakerSocket = PlayerSpeaker;
	//
	// SpeakerRoles.Add(, PlayerField);
	// //Default NPC
	// UDialogueSpeakerSocket* NPCSpeaker =
	// 	CreateDefaultSubobject<UDialogueSpeakerSocket>(
	// 		"DefaultSpeakerNPC"
	// 	);
	// NPCSpeaker->SetSpeakerName("NPC");
	// FSpeakerField NPCField;
	// NPCField.GraphColor = DefaultSpeakerColors.PopColor();
	// NPCField.SpeakerSocket = NPCSpeaker;
	//
	// SpeakerRoles.Add(NPCSpeaker->GetSpeakerName(), NPCField);
}

void UDialogue::OnChangeSpeakers(const EPropertyChangeType::Type& ChangeType)
{
	if (ChangeType == EPropertyChangeType::ArrayAdd)
	{
		OnAddSpeaker();
	}
	else if (ChangeType == EPropertyChangeType::ArrayRemove)
	{
		OnRemoveSpeaker();
	}
	else
	{
		OnChangeSingleSpeaker();
	}

	OnSpeakerRolesChanged.ExecuteIfBound();
}

void UDialogue::OnAddSpeaker()
{
	for (auto& Entry : SpeakerRoles)
	{
		FSpeakerField& Value = Entry.Value;

		//If entry has yet to be filled
		if (!Value.SpeakerSocket)
		{
			//Create a new socket for the entry
			Value.SpeakerSocket =
				NewObject<UDialogueSpeakerSocket>(this);

			//In case the name is not unfilled for whatever reason
			Value.SpeakerSocket->SetSpeakerTag(Entry.Key);

			//Add the default color for the entry 
			Value.GraphColor = DefaultSpeakerColors.PopColor();
		}
	}
}

void UDialogue::OnRemoveSpeaker()
{
	//Stub used later if necessary
}

void UDialogue::OnChangeSingleSpeaker()
{
	//Check for any speakers where the socket doesn't match the name
	for (auto& Entry : SpeakerRoles)
	{
		FSpeakerField& Value = Entry.Value;

		if (Value.SpeakerSocket->GetSpeakerTag() != Entry.Key)
		{
			Value.SpeakerSocket->SetSpeakerTag(Entry.Key);
		}
	}
}

bool UDialogue::CanPlay(ADialogueController* InController, 
	FString& OutErrorMessage) const
{
	if (CompileStatus != EDialogueCompileStatus::Compiled)
	{
		OutErrorMessage = "Dialogue is not compiled.";
		return false;
	}
	if (!InController)
	{
		OutErrorMessage = "No valid controller provided.";
		return false;
	}
	if (!RootNode)
	{
		OutErrorMessage = "Entry node does not exist.";
		return false;
	}

	return true;
}

void UDialogue::FillSpeakers(TArray<FGameplayTag> InSpeakerTags)
{
	Speakers = InSpeakerTags;
}
