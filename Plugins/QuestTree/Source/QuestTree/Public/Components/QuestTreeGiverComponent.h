// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/QuestTreeHelper.h"
#include "GameplayTagContainer.h"
#include "QuestTreeGiverComponent.generated.h"

class UQuestTreeGraph;
class UQuestTreeSubsystem;

UCLASS(ClassGroup = ("Quest Tree"), meta = (BlueprintSpawnableComponent))
class QUESTTREE_API UQuestTreeGiverComponent : public UActorComponent
{
	GENERATED_BODY()

	friend UQuestTreeSubsystem;

public:
	// Sets default values for this component's properties
	UQuestTreeGiverComponent();

protected:

	// ~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End UActorComponent interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
	FGameplayTag QuestGiverTag;

public:

	UFUNCTION(Blueprintpure, Category = "Quest Tree|Quest Giver")
	FGameplayTag GetQuestGiverTag() const { return QuestGiverTag; }

	UFUNCTION(Blueprintpure, Category = "Quest Tree|Quest Giver")
	TMap<FGameplayTag, FQuestTreeDataRecord> GetAllRelaventQuestDataRecords() { return RelaventQuestRecords; }

	UFUNCTION(Blueprintpure, Category = "Quest Tree|Quest Giver", meta = (AutoCreateRefTerm = "InQuestTag"))
	bool GetRelaventQuestRecordByTag(const FGameplayTag& InQuestTag, FQuestTreeDataRecord& OutQuestRecord);

	UFUNCTION()
	virtual void OnQuestStatusUpdatedForQuestData(const FQuestTreeDataRecord& InQuestData, const EQuestTreeStatus& NewStatus, const UQuestTreeManagerComponent* ForQuestManager);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnQuestStatusUpdatedForQuestData"))
	void K2_OnQuestStatusUpdatedForQuestData(const FQuestTreeDataRecord& InQuestRecord, const EQuestTreeStatus& NewStatus, const UQuestTreeManagerComponent* ForQuestManager);

	UPROPERTY(BlueprintAssignable)
	FOnQuestGiverQuestUpdated OnQuestUpdated;

protected:

	UPROPERTY(Transient)
	TMap<FGameplayTag, FQuestTreeDataRecord> RelaventQuestRecords;

};