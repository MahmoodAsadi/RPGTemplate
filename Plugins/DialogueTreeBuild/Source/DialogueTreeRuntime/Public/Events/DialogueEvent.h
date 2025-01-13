// Copyright Zachary Brett, 2024. All rights reserved.

#pragma once

//UE
#include "Events/DialogueEventBase.h"
//Plugin
#include "DialogueSpeakerComponent.h"
//Generated
#include "DialogueEvent.generated.h"

class UDialogue;
class UDialogueSpeakerSocket;

/**
 * An event playable from dialogue. Designed to be implemented in 
 * blueprint. 
 */
UCLASS(Abstract, Blueprintable)
class DIALOGUETREERUNTIME_API UDialogueEvent : public UDialogueEventBase
{
	GENERATED_BODY()

public:
	/** UDialogueEventBase Impl. */
	virtual void PlayEvent() override;
	virtual bool HasAllRequirements() const override;
	virtual FText GetGraphDescription_Implementation() const override;
	/** End UDialogueEventBase */

	/**
	* Checks if the event is valid and has all of the information it needs
	* to compile.
	*
	* @return bool - True if all requisite data is present; false otherwise.
	*/
	UFUNCTION(BlueprintNativeEvent)
	bool IsValidEvent() const;
	virtual bool IsValidEvent_Implementation() const;

	/**
	* Sets the event's speaker. 
	* 
	* @param InSpeaker - UDialogueSpeakerSocket*, the speaker. 
	*/
	void SetSpeaker(UDialogueSpeakerSocket* InSpeaker);

	/**
	* Retrieves the target speaker for the event.
	* 
	* @return UDialogueSpeakerSocket* - the target speaker. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	UDialogueSpeakerSocket* GetSpeakerSocket() const;

	/**
	* Retrieves the list of optional additional speaker sockets for the event.
	* 
	* @return TArray<UDialogueSpeakerSocket*> - the list of optional additional
	* speakers.
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<UDialogueSpeakerSocket*> GetAdditionalSpeakerSockets() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialouge")
	APlayerController* GetPlayer();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialouge")
	AGameStateBase* GetGameState();
protected:
	/**
	* User specified behavior for the event.
	* 
	* @param InSpeaker - FGameplayTag, tag that represent speaker
	* component and actor
	* @param OtherSpeakers - TArray<FGameplayTag>, tag that represent other speakers
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	void OnPlayEvent(FGameplayTag InSpeaker);
	virtual void OnPlayEvent_Implementation(FGameplayTag InSpeaker) {};

protected:
	/** Speaker socket for the speaker the event operates on */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	UDialogueSpeakerSocket* Speaker;

	/** Optional additional speakers to attach to the event */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TArray<UDialogueSpeakerSocket*> AdditionalSpeakers;
};
