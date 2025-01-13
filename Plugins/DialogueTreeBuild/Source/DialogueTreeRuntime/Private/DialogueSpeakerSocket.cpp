// Copyright Zachary Brett, 2024. All rights reserved.

//Header
#include "DialogueSpeakerSocket.h"
//Plugin
#include "Dialogue.h"

void UDialogueSpeakerSocket::SetSpeakerTag(FGameplayTag InTag)
{
	SpeakerTag = InTag;
}

FGameplayTag UDialogueSpeakerSocket::GetSpeakerTag() const
{
	return SpeakerTag;
}

UDialogueSpeakerComponent* UDialogueSpeakerSocket::GetSpeakerComponent(
	UDialogue* InDialogue) const
{
	if (!InDialogue || !SpeakerTag.IsValid())
	{
		return nullptr;
	}

	return nullptr;
}

bool UDialogueSpeakerSocket::IsValidSocket() const
{
	if (!SpeakerTag.IsValid())
	{
		return false;
	}

	return true;
}
