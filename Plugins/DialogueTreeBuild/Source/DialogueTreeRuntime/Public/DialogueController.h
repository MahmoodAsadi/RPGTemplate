// Copyright Zachary Brett, 2024. All rights reserved.

#pragma once

//UE
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//Plugin
#include "Dialogue.h"
//Generated
#include "DialogueController.generated.h"

class UDialogue;
class UDialogueSpeakerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueControllerDelegate, const bool, bIsHUDDialogue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueUIShown, UUserWidget*, DialogueWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FDialogueControllerSpeechDelegate, FSpeechDetails, SpeechDetails);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDialogueActorEmotionReset);
/**
* Struct used to extract node visited data for a single dialogue. 
* Primarily useful for saving/loading.
*/

USTRUCT(BlueprintType)
struct FDialogueNodeVisits
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FName DialogueFName;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Dialogue")
	TSet<int32> VisitedNodeIndices;
};

/**
* Struct used to extract the node visit "memory" of one or more dialogues. 
* Used for saving and loading. 
*/
USTRUCT(BlueprintType)
struct FDialogueRecords
{
	GENERATED_BODY()

	/** Map of dialogue FNames to their records of visited nodes */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Dialogue")
	TMap<FName, FDialogueNodeVisits> Records;
};

/**
* Actor that serves as a controller linking the various aspects of 
* dialogue behavior: user input, dialogue widgets (or other display
* schemes), and the dialogue itself. 
*/
UCLASS(Abstract, notplaceable, BlueprintType, Blueprintable)
class DIALOGUETREERUNTIME_API ADialogueController : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Constructor */
	ADialogueController();

public:
	/**
	* Notifies the dialogue that the user is attempting to select
	* the option at the given index. BlueprintCallable. 
	* 
	* @param InOptionIndex - int32, index of the selection. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void SelectOption(int32 InOptionIndex) const;

	/**
	* Retrieves the array of gameplay tags for the 
	* current dialogue. Empty if the current dialogue is invalid.
	* BlueprintCallable. 
	* 
	* @return TArray<FGameplayTag>, the array of speakers for
	* the current dialogue.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual TArray<FGameplayTag> GetSpeakers() const;

	/**
	* Starts the provided dialogue with the provided
	* speaker gameplay tags. Matches speakers to dialogue roles using
	* the provided name-speaker pairings. 
	* 
	* @param InDialogue - UDialogue*, the dialogue to start. 
	* @param InSpeakerTags - TArray<FGameplayTag>,
	* Speaker tags in dialogue.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogueWithTags(UDialogue* InDialogue, 
		TArray<FGameplayTag> InSpeakerTags, const bool bIsHUDDialogue = true);

	/**
	* Starts the provided dialogue with the provided speaker
	* components.Matches the speakers to dialogue roles using their dialogue 
	* names. Duplicate or unfilled names not allowed.  
	* 
	* @param InDialogue - UDialogue*, the dialogue to start.
	* Components to use.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(UDialogue* InDialogue, const bool bIsHUDDialogue = true);

	/**
	* Starts the provided dialogue with the provided speaker
	* components.Matches the speakers to dialogue roles using their dialogue 
	* names. Duplicate or unfilled names not allowed.  
	* 
	* @param InDialogue - UDialogue*, the dialogue to start.
	* Components to use.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogueWithActor(UDialogue* InDialogue, AActor* InActor);
	
	/**
	* Ends the current dialogue. BlueprintCallable. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	/**
	* Tells the dialogue we want to skip the current speech, if 
	* possible. BlueprintCallable. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Skip() const;
	
	/**
	* Clears all previous visits from the current dialogue's record.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearNodeVisits();

	/**
	* Call this to start the dialogue based on the information that is stored.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartShowingDialog();
	
	/**
	* Exports a dialogue records struct containing the node visits for 
	* all dialogues in the game. Useful for saving.
	* 
	* @return FDialogueMemory - record of all node visits in the game. 
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	FDialogueRecords GetDialogueRecords() const;

	/**
	* Clears node visitation info from all dialogues. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearDialogueRecords();

	/**
	* Imports a dialogue records struct, adding any recorded visits to the 
	* appropriate dialogues. 
	* 
	* @param InRecords - const FDialogueRecords&, the records to load. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ImportDialogueRecords(FDialogueRecords InRecords);

	/**
	* Checks if the specified speaker is a participant in the current dialogue.
	* 
	* @param TargetSpeaker - FGameplayTag
	* @return True if the target speaker is a part of the active dialogue. 
	* False otherwise. 
	*/
	bool SpeakerInCurrentDialogue(FGameplayTag TargetSpeaker) 
		const;

	/**
	* Marks the given node visited in the controller's memory. 
	* 
	* @param TargetDialogue, UDialogue*
	* @param TargetNodeIndex, int32
	*/
	void MarkNodeVisited(UDialogue* TargetDialogue, int32 TargetNodeIndex);

	/**
	* Marks the given node unvisited in the controller's memory.
	*
	* @param TargetDialogue, UDialogue*
	* @param TargetNodeIndex, int32
	*/
	void MarkNodeUnvisited(UDialogue* TargetDialogue, int32 TargetNodeIndex);

	/**
	* Clears all node visits for the given dialogue.
	* 
	* @param TargetDialogue, UDialogue*
	*/
	void ClearAllNodeVisitsForDialogue(UDialogue* TargetDialogue);

	/**
	* Checks if the given node has already been visited. 
	* 
	* @param TargetDialogue, UDialogue*
	* @param TargetNodeIndex, int32
	* @return bool - True if the node was visited, False otherwise. 
	*/
	bool WasNodeVisited(const UDialogue* TargetDialogue, 
		int32 TargetNodeIndex) const;

public:
	/**
	* Opens the user-defined dialogue display.
	* BlueprintImplementable.
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, 
		Category="Dialogue")
	void OpenDisplay();

	/**
	* Closes the user-defined dialogue display. 
	* BlueprintImplementable. 
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable,
		Category="Dialogue")
	void CloseDisplay();

	/**
	* Displays the specified speech in dialogue. 
	* BlueprintImplementable. 
	* 
	* @param InSpeechDetails - FSpeechDetails, struct defining 
	* speech details 
	* @param InSpeaker - FGameplayTag, speaker
	* gameplay tag associated with the target speech. 
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void DisplaySpeech(FSpeechDetails InSpeechDetails,
		FGameplayTag InSpeaker);

	/**
	* Displays a set of options for the user to select from 
	* in dialogue. BlueprintImplementable. 
	* 
	* @param InOptions - const TArray<FSpeechDetails>&, the speech
	* details to display as options. 
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void DisplayOptions(const TArray<FSpeechDetails>& InOptions);

	/**
	* Checks if we can open the user-defined dialogue display. 
	* BlueprintImplementable. 
	*/
	UFUNCTION(BlueprintImplementableEvent)
	bool CanOpenDisplay() const;

	/**
	* User-defined behavior for when a listed speaker component is
	* not provided at dialogue start. 
	* 
	* @param MissingName, const FName&, the name of the missing
	* speaker component. 
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void HandleMissingSpeaker(const FName& MissingName);
	
protected:
	/** The dialogue currently being played. */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	UDialogue* CurrentDialogue = nullptr;

	/** The current speaker's tag. */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FGameplayTag> CurrentSpeakerTags;

	/** The actor the player is speaking to, if there is any */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	AActor* CurrentSpeakerActor;
	
private:
	/** Controller's memory of visited nodes */
	FDialogueRecords DialogueRecords;

	/** Controller's memory of played dialogues */
	UPROPERTY()
	TArray<FString> DialoguePlayed;

public:
	/** Delegate event call for when a new dialogue is started.*/
	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FDialogueControllerDelegate OnDialogueStarted;

	/** Delegate event call for when a dialogue ends.*/
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FDialogueControllerDelegate OnDialogueEnded;

	/** Delegate event call for when a speech plays.*/
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FDialogueControllerSpeechDelegate OnDialogueSpeechDisplayed;

	/** Delegate event call for when an emotion is reset*/
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FDialogueActorEmotionReset OnActorEmotionReset;

	/** Called after the dialogue menu is drawn on screen*/
	UFUNCTION(BlueprintImplementableEvent)
	void PostDialogueMenuShown(UUserWidget* InDialogueWidget);

	/** Check if the dialogue is played before or not */
	UFUNCTION(Blueprintable, BlueprintPure)
	bool CheckIsDialoguePlayed(const FString& InDialogueName) const;

	/** Check if the dialogue is playing now or not*/
	UFUNCTION(Blueprintable, BlueprintPure)
	bool CheckIsDialoguePlaying() const;
};
