// Copyright Pixelation Labs Pte Ltd. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/DialogueSpeechNode.h"
#include "DialogueChoiceNode.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGUETREERUNTIME_API UDialogueChoiceNode : public UDialogueSpeechNode
{
	GENERATED_BODY()
public:
	
	/** DialogueNode Impl. */
	virtual void EnterNode() override;
	//virtual void Skip() override;
	/** End DialogueNode */
};
