// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "QuestTreeManagerComponent.generated.h"

class UQuestTreeGiverComponent;
class UQuestTreeGraph;

UCLASS(ClassGroup = ("Quest Tree"), meta = (BlueprintSpawnableComponent))
class QUESTTREE_API UQuestTreeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQuestTreeManagerComponent();

protected:
	
	// ~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End UActorComponent interface

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Tree")
	TSet<UQuestTreeGraph*> QuestTreeGraphs;

	/*
	* Makes the quest available to start
	*
	* @param InQuestData	Quest information which is available to start.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (HidePin = "bNotifyGraph"))
	void AddAvailableQuest(const FQuestTreeData& InQuestData, bool bNotifyGraph = true);

	/*
	* Activates the quest if not already activated, quest giver calls this after interaction
	*
	* @param InQuestData	Quest information which is available to start.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (HidePin = "bNotifyGraph", AutoCreateRefTerm = "InQuestTag"))
	void ActivateQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Complete the quest
	*
	* @param InQuestTag		Tag of the quest.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (HidePin = "bNotifyGraph", AutoCreateRefTerm = "InQuestTag"))
	void CompleteQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Fail the quest
	*
	* @param InQuestTag		Tag of the quest.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (HidePin = "bNotifyGraph", AutoCreateRefTerm = "InQuestTag"))
	void FailQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Expire the quest, which makes to the quest unable to start again
	*
	* @param InQuestTag		Tag of the quest.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (HidePin = "bNotifyGraph", AutoCreateRefTerm = "InQuestTag"))
	void ExpireQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph = true);

	/*
	* Expire the quest, which makes to the quest unable to start again
	*
	* @param InQuestTag		Tag of the quest.
	* @return				Returns the current state of the quest.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (AutoCreateRefTerm = "InQuestTag"))
	EQuestTreeStatus GetQuestStatus(const FGameplayTag& InQuestTag) const;

	/*
	* Executes event if it was not executed before
	* @param InEventTag		Tag of the event.
	* @param bNotifyGraph	Should be true if called by user to make sure it notifies the registered quest givers from new change, false if triggered from the graph itself to prevent loop.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Manager Component", meta = (HidePin = "bNotifyGraph", AutoCreateRefTerm = "InEventTag"))
	void ExecuteQuestEvent(const FGameplayTag& InEventTag, bool bNotifyGraph = true);

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Manager Component")
	bool DoesActiveQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Manager Component")
	bool DoesCompletedQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Manager Component")
	bool DoesExpiredQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Manager Component")
	bool DoesExecutedEventsMatchesQuery(const FGameplayTagQuery& InEventQuery) const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Manager Component", meta = (AutoCreateRefTerm = "InEventTag"))
	bool IsEventExecuted(const FGameplayTag& InEventTag) const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Manager Component")
	bool IsOwningClient() const;

};