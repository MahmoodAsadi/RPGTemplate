// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestTreeHelper.h"
#include "QuestTreeSubsystem.generated.h"

class UQuestTreeGiverComponent;
class UQuestTreeManagerComponent;
class UQuestTreeGraph;
class UQuestTreeObjective;

/**
 * 
 */
UCLASS()
class QUESTTREE_API UQuestTreeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	friend UQuestTreeManagerComponent;

public:

	static UQuestTreeSubsystem* Get(const UObject* WorldContextObject);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable)
	FOnQuestGiverRegisterationChanged OnQuestGiverRegistered;

	UPROPERTY(BlueprintAssignable)
	FOnQuestGiverRegisterationChanged OnQuestGiverUnregistered;

	UPROPERTY(BlueprintAssignable)
	FOnQuestTreeAnyQuestUpdated OnAnyQuestUpdated;

	UFUNCTION()
	void RegisterQuestGiver(UQuestTreeGiverComponent* InQuestGiver);

	UFUNCTION()
	void UnregisterQuestGiver(UQuestTreeGiverComponent* InQuestGiver);

	UFUNCTION()
	void RegisterQuestManager(UQuestTreeManagerComponent* InQuestManager);

	UFUNCTION()
	void UnregisterQuestManager();

	UFUNCTION()
	TArray<UQuestTreeGiverComponent*> GetRegisteredQuestGivers() { return RegisteredQuestGivers; }
	
	UFUNCTION(BlueprintCallable, Category = "QuestTree Subsystem", meta = (AutoCreateRefTerm = "InSlotName"))
	void SaveLocalQuestDatabaseInSlot(const FString& InSlotName);

	UFUNCTION(BlueprintCallable, Category = "QuestTree Subsystem", meta = (AutoCreateRefTerm = "InSlotName"))
	void LoadLocalQuestDatabaseForProfile(const FString& InSlotName);

protected:

	/*
	* Makes the quest available to start
	*
	* @param InQuestData	Quest information which is available to start.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION()
	void AddAvailableQuest(const FQuestTreeData& InQuestData, bool bNotifyGraph = true);

	/*
	* Activates the quest if not already activated, quest giver calls this after interaction
	*
	* @param InQuestData	Quest information which is available to start.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION()
	void ActivateQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Complete the quest
	*
	* @param InQuestTag		Tag of the quest.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION()
	void CompleteQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Fail the quest
	*
	* @param InQuestTag		Tag of the quest.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION()
	void FailQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Expire the quest, which makes to the quest unable to start again
	*
	* @param InQuestTag		Tag of the quest.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION()
	void ExpireQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Expire the quest, which makes to the quest unable to start again
	*
	* @param InQuestTag		Tag of the quest.
	* @return				Returns the current state of the quest.
	*/
	UFUNCTION()
	EQuestTreeStatus GetQuestStatus(const FGameplayTag& InQuestTag) const;

	/*
	* Executes event if it was not executed before
	* @param InEventTag		Tag of the event.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION()
	void ExecuteQuestEvent(const FGameplayTag& InEventTag, bool bNotifyGraph = true);

	UFUNCTION()
	bool DoesActiveQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const;

	UFUNCTION()
	bool DoesCompletedQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const;

	UFUNCTION()
	bool DoesExpiredQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const;

	UFUNCTION()
	bool DoesExecutedEventsMatchesQuery(const FGameplayTagQuery& InEventQuery) const;

	UFUNCTION()
	bool IsEventExecuted(const FGameplayTag& InEventTag) const;

public:

	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Subsystem", meta = (AutoCreateRefTerm = "InQuestTag"))
	bool FindQuestDataFromQuestGraphsByTag(const FGameplayTag& InQuestTag, FQuestTreeData& FoundQuestData) const;

	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Subsystem")
	bool FindQuestGiversByQuestData(const FQuestTreeData& InQuestData, TArray<UQuestTreeGiverComponent*>& OutQuestGivers);

	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Subsystem", meta = (AutoCreateRefTerm = "InQuestGiversTag"))
	bool FindQuestGiversByTag(const FGameplayTagContainer& InQuestGiversTag, TArray<UQuestTreeGiverComponent*>& OutQuestGivers);

	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Subsystem")
	void ExecuteQuestTrees();

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Subsystem", meta = (AutoCreateRefTerm = "InQuestTag"))
	bool FindQuestsRecordByQuestTag(const FGameplayTag& InQuestTag, FQuestTreeDataRecord& OutQuestRecord) const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Subsystem")
	TArray<FQuestTreeDataRecord> GetAllQuestsRecords() const;

protected:

	// Finds quest data from QuestDataRecords or create one based on the quest data from QuestGraph
	FQuestTreeDataRecord* FindOrAddQuestDataRecordByRef(const FGameplayTag& InQuestTag);

	// Finds quest data from QuestDataRecords, returns null if couldn't find
	FQuestTreeDataRecord* FindQuestDataRecordByRef(const FGameplayTag& InQuestTag);

	// Updates quest status in QuestDataRecords, notify relavent quest givers with updated quest
	void UpdateQuestStatus(const FGameplayTag& InQuestTag, const EQuestTreeStatus& NewStatus);

	// Updates new registered quest giver with relavent QuestDataRecords, Usefull when new level loads or quest giver spawns by game progression.
	void UpdateRelaventQuestRecordsForQuestGiver(UQuestTreeGiverComponent* InQuestGiver);

	// Notify new registered quest giver with relavent QuestData, Usefull when new level loads or quest giver spawns by game progression.
	void UpdateRegisteredQuestGiverStatus(UQuestTreeGiverComponent* InQuestGiver);

	void CreateObjectivesForQuestRecord(FQuestTreeDataRecord* InQuestRecord);
	void LoadObjectivesRecordsForQuestRecord(FQuestTreeDataRecord* InQuestRecord);

	// Resume active quest's objective(s) on game load.
	void ResumeActiveQuestsObjectives();

	void BroadcastAnyQuestUpdate(FQuestTreeDataRecord* InQuestRecord);

	UFUNCTION()
	void OnQuestObjectiveFinished(bool bWasSuccess, const FGameplayTag& QuestTag, UQuestTreeObjective* FinishedObjective);

	UFUNCTION()
	void OnRequestSaveDataInSlot(const FString& InSlotName);

	UFUNCTION()
	void OnRequestLoadDataInSlot(const FString& InSlotName);

	UFUNCTION()
	void StartedLoadingLevel(const FString& InLevelName);

	UFUNCTION()
	void FinishedLoadingLevel(const FString& InLevelName);

	UPROPERTY(Transient)
	bool bQuestDatabaseLoaded = false;

	UPROPERTY(Transient)
	UQuestTreeManagerComponent* OwningQuestManager;

	UPROPERTY(Transient)
	TArray<UQuestTreeGiverComponent*> RegisteredQuestGivers;

	UPROPERTY(Transient)
	TSet<UQuestTreeGraph*> QuestTreeGraphs;

	// Quest data records
	UPROPERTY(Transient)
	TMap<FGameplayTag, FQuestTreeDataRecord> QuestDataRecords;

	// Quests that are available to activate
	UPROPERTY(Transient)
	FGameplayTagContainer AvailableQuests;

	// Quests that are currently active
	UPROPERTY(Transient)
	FGameplayTagContainer ActiveQuests;

	// Quests that are already completed
	UPROPERTY(Transient)
	FGameplayTagContainer CompletedQuests;

	// Quests that are failed
	UPROPERTY(Transient)
	FGameplayTagContainer FailedQuests;

	// Quests that are not available any longer (by user drop it or by story line)
	UPROPERTY(Transient)
	FGameplayTagContainer ExpiredQuests;

	UPROPERTY(Transient)
	FGameplayTagContainer ExecutedEvents;

	UPROPERTY()
	FTimerHandle CleanupQuestObjectivesTimerHandle;
};