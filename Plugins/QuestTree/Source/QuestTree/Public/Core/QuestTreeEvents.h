// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/QuestTreeHelper.h"
#include "Tickable.h"
#include "QuestTreeEvents.generated.h"

class UQuestTreeManagerComponent;
class UQuestTreeGiverComponent;

/**
 * Activatable events, will activate with quest start and ends when quest finishes.
 */
UCLASS(Abstract, EditInlineNew, NotBlueprintable, BlueprintType)
class QUESTTREE_API UQuestTreeActivatableEvent : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	// ~ Start FTickableGameObject Interface
	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override { return false; }
	bool IsTickableWhenPaused() const override { return bTickWhenPaused; }
	TStatId GetStatId() const override { return TStatId(); }
	// ~ End FTickableGameObject Interface

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tick)
	bool bCanTick = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tick)
	bool bTickWhenPaused = false;

	UFUNCTION()
	virtual void OnActivateEvent(UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord);

	UFUNCTION()
	virtual void OnResumeEvent(UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord);

	UFUNCTION()
	virtual void OnDeactivateEvent(UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord);

	// Calls at start of changing level
	UFUNCTION()
	virtual void PauseEvent();

	// Calls after changing level
	UFUNCTION()
	virtual void UnpauseEvent();

	UFUNCTION()
	void PrepareEventForSave(UQuestTreeManagerComponent* InQuestManager);

	UFUNCTION()
	FString GetEventRecordsAsJason(UQuestTreeManagerComponent* InQuestManager);

	UFUNCTION()
	void LoadEventRecordsFromJson(const FString& InJsonString);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "Tick"))
	void K2_Tick(float DeltaTime);

	// Calls when quest or objective activates
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "OnActivateEvent"))
	void K2_OnActivateEvent(const UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord);

	// Calls on loading the game if the event was activate while game was saved
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "OnResumeEvent"))
	void K2_OnResumeEvent(const UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord);

	// Calls when quest or objective deactivates
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "OnDeactivateEvent"))
	void K2_OnDeactivateEvent(const UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord);

	// Calls at start of changing level
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "PauseEvent"))
	void K2_PauseEvent();

	// Calls after changing level
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "UnpauseEvent"))
	void K2_UnpauseEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "PrepareEventForSave"))
	void K2_PrepareEventForSave(UQuestTreeManagerComponent* InQuestManager);

	// bIsPaused will be true only during loading level, Nothing to do with game pause
	UPROPERTY(BlueprintReadOnly, Category = Records)
	bool bIsPaused = false;

};

UCLASS(Abstract, EditInlineNew, Blueprintable)
class QUESTTREE_API UQuestTreeActivatableEvent_Custom : public UQuestTreeActivatableEvent
{
	GENERATED_BODY()

public:

};

/**
 * Executable events will execute once and finish
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class QUESTTREE_API UQuestTreeExecutableEvent : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION()
	virtual void ExecuteEvent(UQuestTreeManagerComponent* InQuestManager) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Events", meta = (DisplayName = "ExecuteEvent"))
	void K2_ExecuteEvent(UQuestTreeManagerComponent* InQuestManager) const;

};