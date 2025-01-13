// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Core/QuestTreeHelper.h"
#include "QuestTreeSaveGame.generated.h"

class UQuestTreeGraph;

/**
 * 
 */
UCLASS()
class QUESTTREE_API UQuestTreeSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Save Data")
	FString SaveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Save Data")
	int64 SaveDateEpoch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	TMap<FGameplayTag, FQuestTreeDataRecord> QuestDataRecords;

	// Quests that are available to activate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	FGameplayTagContainer AvailableQuests;

	// Quests that are currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	FGameplayTagContainer ActiveQuests;

	// Quests that are already completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	FGameplayTagContainer CompletedQuests;

	// Quests that are failed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	FGameplayTagContainer FailedQuests;

	// Quests that are not available any longer (by user drop it or by story line)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	FGameplayTagContainer ExpiredQuests;

	// Executed quest events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	FGameplayTagContainer ExecutedEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestTree Save|Quest Data")
	TArray<TSoftObjectPtr<UQuestTreeGraph>> RegisteredQuestTrees;

};