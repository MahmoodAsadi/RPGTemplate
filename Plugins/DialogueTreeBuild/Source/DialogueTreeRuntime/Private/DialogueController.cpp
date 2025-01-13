// Copyright Zachary Brett, 2024. All rights reserved.

//Header
#include "DialogueController.h"
//Plugin
#include "Dialogue.h"
#include "LogDialogueTree.h"
//Engine
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"


// Sets default values
ADialogueController::ADialogueController()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
}

void ADialogueController::SelectOption(int32 InOptionIndex) const
{
	CurrentDialogue->SelectOption(InOptionIndex);
}

TArray<FGameplayTag> ADialogueController::GetSpeakers() const
{
	if (CurrentDialogue)
	{
		return CurrentDialogue->GetAllSpeakers();
	}

	return TArray<FGameplayTag>();
}

void ADialogueController::StartDialogueWithTags(UDialogue* InDialogue, 
	TArray<FGameplayTag> InSpeakerTags, const bool bIsHUDDialogue)
{
	if (!InDialogue)
	{
		UE_LOG(
			LogDialogueTree,
			Error,
			TEXT("Could not start dialogue. Provided dialogue null.")
		);
		return;
	}

	if (!CanOpenDisplay())
	{
		return;
	}

	CurrentDialogue = InDialogue;
	CurrentDialogue->ContextObject = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	CurrentSpeakerTags = InSpeakerTags;
	OpenDisplay();

	OnDialogueStarted.Broadcast(bIsHUDDialogue);
}

void ADialogueController::StartDialogue(UDialogue* InDialogue, const bool bIsHUDDialogue)
{
	if (!IsValid(InDialogue))
		return;
	
	StartDialogueWithTags(InDialogue, InDialogue->GetAllSpeakers(), bIsHUDDialogue);
}

void ADialogueController::StartDialogueWithActor(UDialogue* InDialogue, AActor* InActor)
{
	CurrentSpeakerActor = InActor;
	StartDialogue(InDialogue);
}

void ADialogueController::EndDialogue()
{
	CloseDisplay();
	
	if (CurrentDialogue)
	{
		//Clear any behavior flags from the speakers and stop speaking
		// for (auto& Tag : CurrentDialogue->GetAllSpeakers())
		// {
		// 	if (Tag.IsValid())
		// 	{
		// 		Entry.Value->Stop();
		// 		Entry.Value->ClearBehaviorFlags();
		// 	}
		// }
		DialoguePlayed.AddUnique(CurrentDialogue->GetName());
		CurrentDialogue->ClearController();
		CurrentDialogue = nullptr;
	}
	if(CurrentSpeakerActor)
	{
		OnActorEmotionReset.Broadcast();
		CurrentSpeakerActor = nullptr;
	}
	OnDialogueEnded.Broadcast(true);
}

void ADialogueController::Skip() const
{
	if (CurrentDialogue)
	{
		CurrentDialogue->Skip();
	}
}

void ADialogueController::ClearNodeVisits()
{
	if (CurrentDialogue)
	{
		ClearAllNodeVisitsForDialogue(CurrentDialogue);
	}
}

void ADialogueController::StartShowingDialog()
{
	CurrentDialogue->OpenDialogue(this, CurrentSpeakerTags);
}

FDialogueRecords ADialogueController::GetDialogueRecords() const
{
	return DialogueRecords;
}

void ADialogueController::ClearDialogueRecords()
{
	DialogueRecords.Records.Empty();
}

void ADialogueController::ImportDialogueRecords(FDialogueRecords InRecords)
{
	DialogueRecords = InRecords;
}

bool ADialogueController::SpeakerInCurrentDialogue(FGameplayTag TargetSpeaker) const
{
	//If no active dialogue, then automatically false
	if (!CurrentDialogue)
	{
		return false;
	}

	//Retrieve speakers
	TArray<FGameplayTag> Speakers =
		CurrentDialogue->GetAllSpeakers();

	//If any one speaker matches the target, true
	for (FGameplayTag NextRole : Speakers)
	{
		if (NextRole == TargetSpeaker)
		{
			return true;
		}
	}

	//No speakers matched the target, false
	return false;
}

void ADialogueController::MarkNodeVisited(UDialogue* TargetDialogue, int32 TargetNodeIndex)
{
	if (!TargetDialogue)
	{
		return;
	}

	FName TargetDialogueName = TargetDialogue->GetFName();

	if (TargetDialogueName.IsEqual(NAME_None))
	{
		return;
	}

	//Create a new record if the target record does not exist
	if (!DialogueRecords.Records.Contains(TargetDialogueName))
	{
		FDialogueNodeVisits NewRecord;
		NewRecord.DialogueFName = TargetDialogueName;

		DialogueRecords.Records.Add(TargetDialogueName, NewRecord);
	}

	//Mark the node visited in the record 
	DialogueRecords.Records[TargetDialogueName].VisitedNodeIndices.Add(
		TargetNodeIndex
	);
}

void ADialogueController::MarkNodeUnvisited(UDialogue* TargetDialogue, int32 TargetNodeIndex)
{
	if (!TargetDialogue)
	{
		return;
	}

	FName TargetDialogueName = TargetDialogue->GetFName();

	//If there is no record of that dialogue, do nothing
	if (!DialogueRecords.Records.Contains(TargetDialogueName))
	{
		return;
	}

	//If there is a record, remove the target index from the visited nodes
	DialogueRecords.Records[TargetDialogueName].VisitedNodeIndices.Remove(
		TargetNodeIndex
	);
}

void ADialogueController::ClearAllNodeVisitsForDialogue(UDialogue* TargetDialogue)
{
	if (!TargetDialogue)
	{
		return;
	}

	FName TargetDialogueName = TargetDialogue->GetFName();

	//If there is no record of that dialogue, do nothing
	if (!DialogueRecords.Records.Contains(TargetDialogueName))
	{
		return;
	}

	DialogueRecords.Records[TargetDialogueName].VisitedNodeIndices.Empty();
}

bool ADialogueController::WasNodeVisited(const UDialogue* TargetDialogue, 
	int32 TargetNodeIndex) const
{
	if (!TargetDialogue)
	{
		return false;
	}

	FName TargetDialogueName = TargetDialogue->GetFName();

	if (!DialogueRecords.Records.Contains(TargetDialogueName))
	{
		return false;
	}

	FDialogueNodeVisits TargetRecord = 
		DialogueRecords.Records[TargetDialogueName];

	return TargetRecord.VisitedNodeIndices.Contains(TargetNodeIndex);
}

bool ADialogueController::CheckIsDialoguePlayed(const FString& InDialogueName) const
{
	return DialoguePlayed.Contains(InDialogueName);
}

bool ADialogueController::CheckIsDialoguePlaying() const
{
	return CurrentDialogue != nullptr;
}
