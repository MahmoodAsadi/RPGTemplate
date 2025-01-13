// Copyright Zachary Brett, 2024. All rights reserved.

#pragma once

//UE
#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "GameplayTagContainer.h"
//Generated
#include "SpeechDetails.generated.h"

/**
* Struct used to pass information about an individual dialogue 
* speech. 
*/
USTRUCT(BlueprintType)
struct FSpeechDetails
{
	GENERATED_BODY()

	/** The text of the speech */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText SpeechText = FText();

	/** The name of the speaker */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FGameplayTag SpeakerTag;

	/** This will be used to decide what animation will be played*/
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (Categories = "Emotion"))
	FGameplayTag EmotionTag;
	
	/** The audio associated with the speech */
	UPROPERTY()
	USoundBase* SpeechAudio = nullptr;

	/** The minimum time for the speech to play before transitioning (unless
	* skipped) */
	// UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	// float MinimumPlayTime = 0.f;

	/** Any behavior flags associated with the speech */
	UPROPERTY()
	FGameplayTagContainer BehaviorFlags;

	/** Whether the speech content should be ignored when entering the node */
	UPROPERTY()
	bool bIgnoreContent = false;

	/** Whether the player is allowed to skip the speech */
	UPROPERTY()
	bool bCanSkip = true;

	/** If true, will only serve as a player's choice and will not
	 * display on the dialogue body.*/
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bIsChoice = false;
};