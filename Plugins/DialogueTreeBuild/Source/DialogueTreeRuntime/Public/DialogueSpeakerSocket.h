// Copyright Zachary Brett, 2024. All rights reserved.

#pragma once

//UE
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
//Generated
#include "DialogueSpeakerSocket.generated.h"

/**
* Object used to represent the notion of a "Speaker" when the actual
* SpeakerComponent may not be immediately available. 
*/
UCLASS()
class DIALOGUETREERUNTIME_API UDialogueSpeakerSocket : public UObject
{
	GENERATED_BODY()

public:
	/**
	* Sets the speaker's tag to the provided value.
	* 
	* @param InTag - FGameplayTag, the new tag 
	*/
	void SetSpeakerTag(FGameplayTag InTag);

	/**
	* Retrieves the speaker's tag. 
	* 
	* @return FGameplayTag, the speaker's tag
	*/
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	FGameplayTag GetSpeakerTag() const;

	/**
	* Retrieve the component associated with this speaker from 
	* the provided dialogue. 
	* 
	* @param InDialogue*, UDialogue* to get the speaker component from.
	* @return UDialogueSpeakerComponent*, the component for the speaker in the 
	* given dialogue or nullptr if none found.
	*/
	class UDialogueSpeakerComponent* GetSpeakerComponent(
		class UDialogue* InDialogue) const;

	/**
	* Checks to see if the socket's value is valid. 
	* 
	* @return true if the socket's value is valid, false otherwise
	*/
	bool IsValidSocket() const;

private:
	/** Name of the speaker */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FGameplayTag SpeakerTag;
};
