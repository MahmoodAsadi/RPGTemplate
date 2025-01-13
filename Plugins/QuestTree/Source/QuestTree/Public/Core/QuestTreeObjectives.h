// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestTreeHelper.h"
#include "Tickable.h"
#include "TimerManager.h"
#include "QuestTreeObjectives.generated.h"

class UQuestTreeManagerComponent;
class UQuestTreeActivatableEvent;

USTRUCT(BlueprintType)
struct FTimeBasedObjectiveRecord
{
	GENERATED_BODY()

public:

	virtual ~FTimeBasedObjectiveRecord() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RecordInfo)
	float RemainingTime = -1.0f;

	FString JsonString;
	
	FTimeBasedObjectiveRecord() {}
	FTimeBasedObjectiveRecord(const FString& InJasonString);
};

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType)
class QUESTTREE_API UQuestTreeObjective : public UObject, public FTickableGameObject
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

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree|Objectives", meta = (DisplayName = "Tick"))
	void K2_Tick(float DeltaTime);

	// Activates objective for quest manager by given quest record.
	UFUNCTION()
	virtual void ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord);

	// Resume objective from loading game or chaning level with saved data in "Records" struct property.
	UFUNCTION()
	virtual void ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord);

	// Calls at start of changing level
	UFUNCTION()
	virtual void PauseObjective();

	// Calls after changing level
	UFUNCTION()
	virtual void UnPauseObjective();

	// Finishes objective
	UFUNCTION()
	virtual void FinishedObjective(bool bWasSuccess);

	// Finishes this objecive with the given result.
	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Objectives")
	void FinishObjective(bool bSuccess);

	// Convert data stored in "Records" struct property to json string
	UFUNCTION()
	FString GetObjectiveRecordsAsJson(UQuestTreeManagerComponent* InQuestManager);

	UFUNCTION()
	FString GetEventRecordsAsJason(UQuestTreeManagerComponent* InQuestManager);

	// Convert json string to "Records" struct property
	UFUNCTION()
	void LoadObjectiveRecordsFromJson(const FString& InJsonString);

	UFUNCTION()
	void LoadEventRecordsFromJson(const FString& InJsonString);

	// Preparing objective for being saved, Cache necessary properties into "Records" property struct.
	UFUNCTION()
	virtual void PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager);

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Objectives")
	EQuestTreeObjectiveStatus GetObjectiveStatus() const { return ObjectiveStatus; }

	/*
	* Returns quest records cached at the activation of this quest.
	* 
	* Note: The actual quest record during this objective might be different from initial record.
	* Better to get the current record from QuestTreeSubsystem.
	*/
	UFUNCTION(BlueprintPure, Category = "Quest Tree|Objectives")
	FQuestTreeDataRecord GetQuestRecord() const { return QuestRecord; }

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Objectives")
	bool DoesFailQuestIfUnsuccessful() const { return bFailQuestIfUnsuccessful; }

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Quest Tree|Objectives")
	void GetObjectiveCounter(int32& CurrentCount, int32& MaxCount);
	virtual void GetObjectiveCounter_Implementation(int32& CurrentCount, int32& MaxCount) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Quest Tree|Objectives")
	bool GetHasTimer();
	virtual bool GetHasTimer_Implementation() { return false; }

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Quest Tree|Objectives")
	float GetRemainingTime();
	virtual float GetRemainingTime_Implementation() { return -1.0f; }

	UPROPERTY(BlueprintAssignable)
	FOnQuestTreeObjectiveFinished OnObjectiveFinished;

	/*
	* This objective can tick?
	* 
	* Note: If true, objective can only tick while it is in progress.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tick)
	bool bCanTick = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tick)
	bool bTickWhenPaused = false;

	/*
	* GameplayTag for this objective
	* 
	* Note: Different questes can share same ObjectiveTag, but a quest can not share same ObjectiveTag for different objectives.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Objective)
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Objective)
	FGameplayTagContainer ObjectiveContextTags;

	// Short brief of current objective.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Objective, meta = (MultiLine = "true"))
	FText ObjectiveBrief;

	UPROPERTY(BlueprintReadOnly, Category = Objective)
	bool bHasCounter = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Objective, meta = (EditCondition = "bHasCounter"))
	TArray<FQuestTreeObjectiveCounterInfo> ObjectiveCounterInfo;

	/*
	* If true, the owning quest will fail if finished this objective unsuccessful, otherwise will proceed to next objective.
	* 
	* Note: Make sure it is true for last objective for the owning quest since there will be no objective left to proceed to.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Objective)
	bool bFailQuestIfUnsuccessful = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = Event)
	UQuestTreeActivatableEvent* ObjectiveEvent;

	UPROPERTY(BlueprintReadOnly, Category = Records)
	EQuestTreeObjectiveStatus ObjectiveStatus;
	
	// The quest Record when objective was activated.
	UPROPERTY(BlueprintReadOnly, Category = Records)
	FQuestTreeDataRecord QuestRecord;

	UPROPERTY(BlueprintReadOnly, Category = Records)
	FGameplayTag OwningQuestTag;

	// bIsPaused will be true only during loading level, Nothing to do with game pause
	UPROPERTY(BlueprintReadOnly, Category = Records)
	bool bIsPaused = false;

	UPROPERTY()
	UQuestTreeManagerComponent* OwningQuestManager;

	UFUNCTION()
	void StartedLoadingLevel(const FString& InLevelName);

	UFUNCTION()
	void FinishedLoadingLevel(const FString& InLevelName);

};

/**
 * Auto complete quest by given duration
 */
UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "Wait"))
class QUESTTREE_API UQuestTreeObjective_Wait : public UQuestTreeObjective
{
	GENERATED_BODY()

public:

	virtual void ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord) override;
	virtual void ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord) override;
	virtual void PauseObjective() override;
	virtual void UnPauseObjective() override;
	virtual void PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager) override;
	virtual bool GetHasTimer_Implementation() override { return true; }
	virtual float GetRemainingTime_Implementation() override;

protected:

	// Duration until objective compelete
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wait, meta = (Units = Seconds, ClampMin = 0.0f, UIMin = 0.0f))
	float Duration = 5.0f;

	UPROPERTY(Transient)
	FTimerHandle ObjectiveTimerHandle;

	UPROPERTY(BlueprintReadOnly, Category = Records)
	FTimeBasedObjectiveRecord Records;

};

/**
 * Fails quest when countdown reaches zero
 */
UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "Countdown"))
class QUESTTREE_API UQuestTreeObjective_CountDown : public UQuestTreeObjective
{
	GENERATED_BODY()

public:

	virtual void ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord) override;
	virtual void ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord) override;
	virtual void PauseObjective() override;
	virtual void UnPauseObjective() override;
	virtual void PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager) override;
	virtual bool GetHasTimer_Implementation() override { return true; }
	virtual float GetRemainingTime_Implementation() override;

protected:

	// Duration until objective fail
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wait, meta = (Units = Seconds, ClampMin = 0.0f, UIMin = 0.0f))
	float Duration = 5.0f;

	UPROPERTY(Transient)
	FTimerHandle ObjectiveTimerHandle;

	UPROPERTY(BlueprintReadOnly, Category = Records)
	FTimeBasedObjectiveRecord Records;

};

/**
 * Custom class to create quest tree objective blueprint
 * 
 * Note: Make sure to create a struct (any type) with name "Records" and keep current state of the objective inside it
 * Only keep value types in the struct, UObjects will not be saved
 * 
 * You need to set the information you need to save in "Records" struct, by overriding "PrepareObjectiveForSave" function.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class QUESTTREE_API UQuestTreeObjective_Custom : public UQuestTreeObjective
{
	GENERATED_BODY()

public:

	virtual void ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord) override;
	virtual void ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord) override;
	virtual void FinishedObjective(bool bWasSuccess) override;
	virtual void PauseObjective() override;
	virtual void UnPauseObjective() override;
	virtual void PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager) override;

	// Calls when objective activated for first time.
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree Objective", meta = (DisplayName = "ActivateObjective"))
	void K2_ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord);

	// Calls when resume objective from loading game or chaning level with saved data in "Records" struct property.
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree Objective", meta = (DisplayName = "ResumeObjective"))
	void K2_ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree Objective", meta = (DisplayName = "FinishedObjective"))
	void K2_FinishedObjective(bool bWasSuccess);

	/*
	* Preparing objective for being saved, Cache necessary data (properties) into "Records" property struct.
	* 
	* Note: 'Records' should be any type of struct, make sure the properties in the struct are not 'Transient' otherwise may not save properly.
	* UObjects will not be saved, make sure to not add them to the struct.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree Objective", meta = (DisplayName = "PrepareObjectiveForSave"))
	void K2_PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager);

	// Calls at start of changing level
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree Objective", meta = (DisplayName = "PauseObjective"))
	void K2_PauseObjective();

	// Calls after changing level
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tree Objective", meta = (DisplayName = "UnPauseObjective"))
	void K2_UnPauseObjective();

};