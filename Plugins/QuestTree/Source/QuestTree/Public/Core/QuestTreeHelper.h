// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Interfaces/IHttpResponse.h"
#include "QuestTreeHelper.generated.h"

class UEdGraphNode;
class UQuestTreeActivatableEvent;
class UQuestTreeGiverComponent;
class UQuestTreeManagerComponent;
class UQuestTreeObjective;
class UWidget;

UENUM(BlueprintType)
enum class EQuestTreeStatus : uint8
{
	// Haven't took this quest yet and it is not available yet
	None,

	Available,

	InProgress,

	Completed,

	// Failed the quest
	Failed,

	// Can not take this quest any longer, it failed or the event finished for this quest
	Expired
};

UENUM(BlueprintType)
enum class EQuestTreeObjectiveStatus : uint8
{
	None,

	InProgress,

	Success,

	Failed
};

UENUM()
enum class EQuestTreeCompileStatus : uint8
{
	Compiled,
	Uncompiled,
	Warning,
	Failed
};

USTRUCT()
struct FQuestTreeCompileErrorInfo
{
	GENERATED_BODY()

public:

	FQuestTreeCompileErrorInfo() {}
	FQuestTreeCompileErrorInfo(EQuestTreeCompileStatus InStatus, FText InDescription)
		: Status(InStatus), IssueDescription(InDescription)
	{ }

	UPROPERTY()
	EQuestTreeCompileStatus Status = EQuestTreeCompileStatus::Uncompiled;

	UPROPERTY(meta = (MultiLine = "true"))
	FText IssueDescription;

	UPROPERTY()
	const UEdGraphNode* OwningNode;
};

UENUM(BlueprintType)
enum class EQuestTreeType : uint8
{
	// Main Quest
	MainQuest		UMETA(DisplayName = "Main Quest"),

	// Side Quest
	SideQuest		UMETA(DisplayName = "Side Quest"),

	// Daily Quest
	DailyQuest		UMETA(DisplayName = "Daily Quest")
	//etc.
};

USTRUCT(BlueprintType)
struct FQuestTreeData : public FTableRowBase
{
	GENERATED_BODY()

public:

	FQuestTreeData() {}
	FQuestTreeData(const FGameplayTag& InQuestTag)
		: QuestTag(InQuestTag)
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	FGameplayTag QuestTag;

	// Relavent quest giver(s) to this quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	FGameplayTagContainer QuestGiverTag;

	// Represent type of the quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	EQuestTreeType QuestType = EQuestTreeType::MainQuest;

	// If true, automatically starts the quest when available (Conditions met)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	bool bAutoStart = false;

	// Quest title
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest, meta = (MultiLine = "true"))
	FText Title;

	// A brief description of the quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest, meta = (MultiLine = "true"))
	FText Brief;

	// Full description of the quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest, meta = (MultiLine = "true"))
	FText Description;

	// Objectives of the quest
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = Objectives)
	TArray<UQuestTreeObjective*> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = Event)
	UQuestTreeActivatableEvent* Event;

};

USTRUCT(BlueprintType)
struct FQuestTreeDataRecord : public FTableRowBase
{
	GENERATED_BODY()

public:

	FQuestTreeDataRecord() {}
	FQuestTreeDataRecord(const FQuestTreeData& InQuestData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	FGameplayTag QuestTag;

	// Relavent quest giver(s) to this quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	FGameplayTagContainer QuestGiverTag;

	// Current status of the quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	EQuestTreeStatus QuestStatus = EQuestTreeStatus::None;

	UPROPERTY(BlueprintReadOnly, Transient, Category = Records)
	TArray<UQuestTreeObjective*> Objectives;

	UPROPERTY(BlueprintReadOnly, Transient, Category = Records)
	UQuestTreeActivatableEvent* QuestEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	FString QuestEventJsonData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	TMap<FGameplayTag, EQuestTreeObjectiveStatus> ObjectivesStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	TMap<FGameplayTag, FString> ObjectivesJsonData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Records)
	TMap<FGameplayTag, FString> ObjectivesEventJsonData;

	UPROPERTY(BlueprintReadOnly, Transient, Category = Records)
	bool bResumeQuest = false;

};

USTRUCT(BlueprintType)
struct FQuestTreeObjectiveCounterInfo : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CounterInfo, meta = (MultiLine = "true"))
	FText Brief;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CounterInfo)
	int32 MaxCount = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CounterInfo)
	int32 CurrentCount = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CounterInfo)
	FGameplayTagContainer ContextTags;

};


/**
 * 
 */
UCLASS()
class QUESTTREE_API UQuestTreeHelper : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static void ClearAndInvalidateTimerByHandle(const UObject* Object, UPARAM(ref) FTimerHandle& Handle);

	UFUNCTION(BlueprintPure, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static bool DoesTimerExistsByHandle(const UObject* Object, FTimerHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static float GetTimerElapsedTimeByHandle(const UObject* Object, FTimerHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static float GetTimerRemainingTimeByHandle(const UObject* Object, FTimerHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static bool IsTimerActiveByHandle(const UObject* Object, FTimerHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static bool IsTimerPausedByHandle(const UObject* Object, FTimerHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static void PauseTimerByHandle(const UObject* Object, FTimerHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "Quest Tree Helpers", meta = (DefaultToSelf = "Object"))
	static void UnPauseTimerByHandle(const UObject* Object, FTimerHandle Handle);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestGiverRegisterationChanged, UQuestTreeGiverComponent*, InQuestGiver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestTreeObjectiveFinished, bool, bWasSuccess, const FGameplayTag&, QuestTag, UQuestTreeObjective*, FinishedObjective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestTreeAnyQuestUpdated, const FQuestTreeDataRecord&, InQuestRecord, EQuestTreeStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnQuestGiverQuestUpdated, const FQuestTreeDataRecord&, InQuestData, EQuestTreeStatus, NewStatus, const UQuestTreeManagerComponent*, ForQuestManager, UQuestTreeGiverComponent*, InQuestGiver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestTreeObjectiveActivates, const FQuestTreeDataRecord&, InQuestRecord, UQuestTreeObjective*, InObjective);