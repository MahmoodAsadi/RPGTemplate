// Copyright Pixelation Labs Pte Ltd. All rights reserved.


#include "Graph/Nodes/GraphNodeDialogueChoice.h"

#include "Dialogue.h"
#include "DialogueSpeakerSocket.h"
#include "Nodes/DialogueChoiceNode.h"
#include "Transitions/AutoDialogueTransition.h"
#include "Transitions/DialogueTransition.h"

UGraphNodeDialogueSpeech* UGraphNodeDialogueChoice::MakeTemplate(UObject* Outer, UDialogueSpeakerSocket* InSpeaker)
{
	check(InSpeaker && Outer);

	UGraphNodeDialogueChoice* NewSpeech =
		NewObject<UGraphNodeDialogueChoice>(Outer);
	NewSpeech->SetSpeaker(InSpeaker);
	NewSpeech->SetTransitionType(UAutoDialogueTransition::StaticClass());
	NewSpeech->bIsChoice = true;
	
	return NewSpeech;
}

void UGraphNodeDialogueChoice::CreateAssetNode(UDialogue* InAsset)
{
	check(Speaker.Speaker);
	check(TransitionType);

	//Create node
	UDialogueChoiceNode* NewNode = 
		NewObject<UDialogueChoiceNode>(InAsset);
	SetAssetNode(NewNode);
    
	//Init data 
	FSpeechDetails SpeechDetails;
	SpeechDetails.SpeakerTag = Speaker.Speaker->GetSpeakerTag();
	SpeechDetails.bIgnoreContent = bIgnoreContent;
	SpeechDetails.SpeechText = SpeechText;
	SpeechDetails.SpeechAudio = SpeechAudio;
	// SpeechDetails.MinimumPlayTime = MinimumPlayTime;
	SpeechDetails.EmotionTag = SpeechEmotionTag;
	SpeechDetails.bIsChoice = bIsChoice;
	SpeechDetails.bCanSkip = bCanSkip;
	SpeechDetails.BehaviorFlags = BehaviorFlags;

	NewNode->InitSpeechData(SpeechDetails, TransitionType);

	//Add node to the dialogue
	InAsset->AddNode(NewNode);
}

FLinearColor UGraphNodeDialogueChoice::GetNodeTitleColor() const
{
	return FLinearColor::Black;
}
