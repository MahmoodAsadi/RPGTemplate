// Copyright Pixelation Labs Pte Ltd. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GraphNodeDialogueSpeech.h"
#include "GraphNodeDialogueChoice.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGUETREEEDITOR_API UGraphNodeDialogueChoice : public UGraphNodeDialogueSpeech
{
	GENERATED_BODY()
public:
	/**
	* Pseudo-Constructor used to instantiate a new speech node template. Static.
	* 
	* @param Outer - UObject*, the owning object/graph for the node. 
	* @param InSpeaker - UDialogueSpeakerSocket*, the node's speaker. 
	*/
	static UGraphNodeDialogueSpeech* MakeTemplate(UObject* Outer,
		UDialogueSpeakerSocket* InSpeaker);
	
	/** UGraphNodeDialogue Implementation */
	virtual void CreateAssetNode(class UDialogue* InAsset) override;
	/** End UGraphNodeDialogue */

	
	/** UEdGraphNode Implementation */
	virtual FLinearColor GetNodeTitleColor() const override;
	/** End UEdGraphNode */
};
