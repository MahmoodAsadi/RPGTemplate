// Copyright Zachary Brett, 2024. All rights reserved.

//Header
#include "Events/DialogueEvent.h"
//Plugin
#include "Dialogue.h"
#include "DialogueManagerSubsystem.h"
#include "DialogueSpeakerComponent.h"
#include "DialogueSpeakerSocket.h"
#include "LogDialogueTree.h"
#include "GameFramework/GameStateBase.h"

void UDialogueEvent::PlayEvent()
{
	check(Dialogue && Speaker);

	FGameplayTag SpeakerTag =
		Speaker->GetSpeakerTag();

	if (!SpeakerTag.IsValid())
	{
		UE_LOG(
			LogDialogueTree, 
			Warning,
			TEXT("Failed to play event because the target speaker tag was not supplied. Verify that the dialogue name property matches the speaker's role in the dialogue.")
		);
		return;
	}

	TArray<FGameplayTag> OtherSpeakers;
	for (UDialogueSpeakerSocket* Socket : AdditionalSpeakers)
	{
		if (!Socket)
		{
			UE_LOG(
				LogDialogueTree,
				Warning,
				TEXT("Failed to play event because the target speaker component was not supplied. Verify that the dialogue name property matches the speaker's role in the dialogue.")
			);
			return;
		}
		
		OtherSpeakers.Add(Socket->GetSpeakerTag());
	}

	OnPlayEvent(SpeakerTag);
}

bool UDialogueEvent::HasAllRequirements() const
{
	return Speaker != nullptr && IsValidEvent();
}

FText UDialogueEvent::GetGraphDescription_Implementation() const
{
	return FText::FromString(GetClass()->GetName());
}

bool UDialogueEvent::IsValidEvent_Implementation() const
{
	return true;
}

void UDialogueEvent::SetSpeaker(UDialogueSpeakerSocket* InSpeaker)
{
	Speaker = InSpeaker;
}

UDialogueSpeakerSocket* UDialogueEvent::GetSpeakerSocket() const
{
	return Speaker;
}

TArray<UDialogueSpeakerSocket*> UDialogueEvent::GetAdditionalSpeakerSockets() const
{
	return AdditionalSpeakers;
}

APlayerController* UDialogueEvent::GetPlayer()
{
	if(Dialogue)
	{
		if(Dialogue && Dialogue->ContextObject)
		{
			APlayerController* pc = Cast<APlayerController>(Dialogue->ContextObject);
			return pc;
		}
	}
	
	return nullptr;
}

AGameStateBase* UDialogueEvent::GetGameState()
{
	if(Dialogue)
	{
		if(Dialogue && Dialogue->ContextObject && Dialogue->ContextObject->GetWorld())
		{
			AGameStateBase* gs = Cast<AGameStateBase>(Dialogue->ContextObject->GetWorld()->GetGameState());
			return gs;
		}
	}
	return nullptr;
}
