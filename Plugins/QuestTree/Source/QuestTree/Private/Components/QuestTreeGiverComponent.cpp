// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/QuestTreeGiverComponent.h"
#include "Core/QuestTreeSubsystem.h"
#include "Graph/QuestTreeGraph.h"


// Sets default values for this component's properties
UQuestTreeGiverComponent::UQuestTreeGiverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UQuestTreeGiverComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this))
	{
		QuestManagerSubsystem->RegisterQuestGiver(this);
	}
}

void UQuestTreeGiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this))
	{
		QuestManagerSubsystem->UnregisterQuestGiver(this);
	}

	Super::EndPlay(EndPlayReason);
}

bool UQuestTreeGiverComponent::GetRelaventQuestRecordByTag(const FGameplayTag& InQuestTag, FQuestTreeDataRecord& OutQuestRecord)
{
	if (RelaventQuestRecords.Contains(InQuestTag))
	{
		OutQuestRecord = RelaventQuestRecords[InQuestTag];
		return true;
	}
	
	return false;
}

void UQuestTreeGiverComponent::OnQuestStatusUpdatedForQuestData(const FQuestTreeDataRecord& InQuestData, const EQuestTreeStatus& NewStatus, const UQuestTreeManagerComponent* ForQuestManager)
{
	if (RelaventQuestRecords.Contains(InQuestData.QuestTag))
		RelaventQuestRecords[InQuestData.QuestTag] = InQuestData;
	else
		RelaventQuestRecords.Add(InQuestData.QuestTag, InQuestData);

	K2_OnQuestStatusUpdatedForQuestData(InQuestData, NewStatus, ForQuestManager);
	OnQuestUpdated.Broadcast(InQuestData, NewStatus, ForQuestManager, this);
}