// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/QuestTreeSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "Components/QuestTreeGiverComponent.h"
#include "Components/QuestTreeManagerComponent.h"
#include "Core/QuestTreeObjectives.h"
#include "Core/QuestTreeSaveGame.h"
#include "Core/QuestTreeEvents.h"
#include "Graph/QuestTreeGraph.h"
#include "ProfileManagerSubsystem.h"


UQuestTreeSubsystem* UQuestTreeSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();
	return World->GetGameInstance()->GetSubsystem<UQuestTreeSubsystem>();
}

bool UQuestTreeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void UQuestTreeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Collection.InitializeDependency(UProfileManagerSubsystem::StaticClass());

	if (UProfileManagerSubsystem* ProfileManagerSubsystem = UProfileManagerSubsystem::Get(this))
	{
		ProfileManagerSubsystem->OnRequestSaveProfile.AddUniqueDynamic(this, &UQuestTreeSubsystem::OnRequestSaveDataInSlot);
		ProfileManagerSubsystem->OnRequestLoadProfile.AddUniqueDynamic(this, &UQuestTreeSubsystem::OnRequestLoadDataInSlot);
		ProfileManagerSubsystem->OnStartLoadingLevel.AddUniqueDynamic(this, &UQuestTreeSubsystem::StartedLoadingLevel);
		ProfileManagerSubsystem->OnLoadingLevelFinished.AddUniqueDynamic(this, &UQuestTreeSubsystem::FinishedLoadingLevel);
	}
}

void UQuestTreeSubsystem::Deinitialize()
{
	if (UProfileManagerSubsystem* ProfileManagerSubsystem = UProfileManagerSubsystem::Get(this))
	{
		ProfileManagerSubsystem->OnRequestSaveProfile.RemoveDynamic(this, &UQuestTreeSubsystem::OnRequestSaveDataInSlot);
		ProfileManagerSubsystem->OnRequestLoadProfile.RemoveDynamic(this, &UQuestTreeSubsystem::OnRequestLoadDataInSlot);
		ProfileManagerSubsystem->OnStartLoadingLevel.RemoveDynamic(this, &UQuestTreeSubsystem::StartedLoadingLevel);
		ProfileManagerSubsystem->OnLoadingLevelFinished.RemoveDynamic(this, &UQuestTreeSubsystem::FinishedLoadingLevel);
	}

	Super::Deinitialize();
}

void UQuestTreeSubsystem::RegisterQuestGiver(UQuestTreeGiverComponent* InQuestGiver)
{
	if (!IsValid(InQuestGiver))
		return;

	if (!RegisteredQuestGivers.Contains(InQuestGiver))
	{
		RegisteredQuestGivers.AddUnique(InQuestGiver);
		OnQuestGiverRegistered.Broadcast(InQuestGiver);
		UpdateRelaventQuestRecordsForQuestGiver(InQuestGiver);
		
		if (IsValid(OwningQuestManager))
			UpdateRegisteredQuestGiverStatus(InQuestGiver);
	}
}

void UQuestTreeSubsystem::UnregisterQuestGiver(UQuestTreeGiverComponent* InQuestGiver)
{
	if (RegisteredQuestGivers.Contains(InQuestGiver))
	{
		RegisteredQuestGivers.Remove(InQuestGiver);
		OnQuestGiverUnregistered.Broadcast(InQuestGiver);
	}
}

void UQuestTreeSubsystem::RegisterQuestManager(UQuestTreeManagerComponent* InQuestManager)
{
	OwningQuestManager = InQuestManager;
	QuestTreeGraphs = InQuestManager->QuestTreeGraphs;
	ExecuteQuestTrees();
	
	for (UQuestTreeGiverComponent* QuestGiver : RegisteredQuestGivers)
		UpdateRegisteredQuestGiverStatus(QuestGiver);

	ResumeActiveQuestsObjectives();
}

void UQuestTreeSubsystem::UnregisterQuestManager()
{
	OwningQuestManager = nullptr;
	QuestTreeGraphs.Empty();
}

void UQuestTreeSubsystem::SaveLocalQuestDatabaseInSlot(const FString& InSlotName)
{
	USaveGame* SaveGame = UGameplayStatics::CreateSaveGameObject(UQuestTreeSaveGame::StaticClass());
	UQuestTreeSaveGame* QuestTreeSave = Cast<UQuestTreeSaveGame>(SaveGame);
	
	for (TPair<FGameplayTag, FQuestTreeDataRecord>& QuestRecord : QuestDataRecords)
	{
		for (UQuestTreeObjective* Objective : QuestRecord.Value.Objectives)
		{
			if (Objective && Objective->GetObjectiveStatus() == EQuestTreeObjectiveStatus::InProgress)
			{
				const FGameplayTag& ObjectiveTag = Objective->ObjectiveTag;
				const FString& ObjectiveRecordJason = Objective->GetObjectiveRecordsAsJson(OwningQuestManager);
				if (!ObjectiveRecordJason.IsEmpty())
					QuestRecord.Value.ObjectivesJsonData.Add(ObjectiveTag, ObjectiveRecordJason);
				
				const FString& ObjectiveEventRecordJason = Objective->GetEventRecordsAsJason(OwningQuestManager);
				if (!ObjectiveEventRecordJason.IsEmpty())
					QuestRecord.Value.ObjectivesEventJsonData.Add(ObjectiveTag, ObjectiveEventRecordJason);
			}
		}

		if (QuestRecord.Value.QuestEvent && QuestRecord.Value.QuestStatus == EQuestTreeStatus::InProgress)
			QuestRecord.Value.QuestEventJsonData = QuestRecord.Value.QuestEvent->GetEventRecordsAsJason(OwningQuestManager);
	}

	QuestTreeSave->RegisteredQuestTrees.Empty();
	for (UQuestTreeGraph* QuestTreeGraph : QuestTreeGraphs)
		QuestTreeSave->RegisteredQuestTrees.Add(QuestTreeGraph);

	QuestTreeSave->QuestDataRecords = QuestDataRecords;
	QuestTreeSave->AvailableQuests = AvailableQuests;
	QuestTreeSave->ActiveQuests = ActiveQuests;
	QuestTreeSave->CompletedQuests = CompletedQuests;
	QuestTreeSave->FailedQuests = FailedQuests;
	QuestTreeSave->ExpiredQuests = ExpiredQuests;
	QuestTreeSave->ExecutedEvents = ExecutedEvents;
	UGameplayStatics::SaveGameToSlot(QuestTreeSave, InSlotName, 0);
}

void UQuestTreeSubsystem::LoadLocalQuestDatabaseForProfile(const FString& InSlotName)
{
	if (UGameplayStatics::DoesSaveGameExist(InSlotName, 0))
	{
		USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(InSlotName, 0);
		UQuestTreeSaveGame* QuestTreeSave = Cast<UQuestTreeSaveGame>(SaveGame);
		QuestDataRecords = QuestTreeSave->QuestDataRecords;
		AvailableQuests = QuestTreeSave->AvailableQuests;
		ActiveQuests = QuestTreeSave->ActiveQuests;
		CompletedQuests = QuestTreeSave->CompletedQuests;
		FailedQuests = QuestTreeSave->FailedQuests;
		ExpiredQuests = QuestTreeSave->ExpiredQuests;
		ExecutedEvents = QuestTreeSave->ExecutedEvents;
		for (TSoftObjectPtr<UQuestTreeGraph> QuestTree : QuestTreeSave->RegisteredQuestTrees)
		{
			QuestTreeGraphs.Add(QuestTree.LoadSynchronous());
		}
		
		// Set resume quest status for records.
		for (TPair<FGameplayTag, FQuestTreeDataRecord>& QuestDataRecord : QuestDataRecords)
		{
			QuestDataRecord.Value.bResumeQuest = QuestDataRecord.Value.QuestStatus == EQuestTreeStatus::InProgress;
		}
	}
}

void UQuestTreeSubsystem::AddAvailableQuest(const FQuestTreeData& InQuestData, bool bNotifyGraph)
{
	const FGameplayTag& QuestTag = InQuestData.QuestTag;

	if (AvailableQuests.HasTagExact(QuestTag)
		|| ActiveQuests.HasTagExact(QuestTag)
		|| CompletedQuests.HasTagExact(QuestTag)
		|| ExpiredQuests.HasTagExact(QuestTag))
		return;

	FailedQuests.RemoveTag(QuestTag);

	if (InQuestData.bAutoStart)
	{
		FindOrAddQuestDataRecordByRef(QuestTag);
		ActivateQuest(QuestTag, bNotifyGraph);
		return;
	}

	AvailableQuests.AddTag(QuestTag);
	FindOrAddQuestDataRecordByRef(QuestTag);
	UpdateQuestStatus(QuestTag, EQuestTreeStatus::Available);

	if (bNotifyGraph)
		ExecuteQuestTrees();
}

void UQuestTreeSubsystem::ActivateQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (ExpiredQuests.HasTagExact(InQuestTag) || ActiveQuests.HasTagExact(InQuestTag))
		return;

	AvailableQuests.RemoveTag(InQuestTag);
	ActiveQuests.AddTag(InQuestTag);
	UpdateQuestStatus(InQuestTag, EQuestTreeStatus::InProgress);
	
	if (bNotifyGraph)
		ExecuteQuestTrees();
}

void UQuestTreeSubsystem::CompleteQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (ExpiredQuests.HasTagExact(InQuestTag) || !ActiveQuests.HasTagExact(InQuestTag) || CompletedQuests.HasTagExact(InQuestTag))
		return;

	ActiveQuests.RemoveTag(InQuestTag);
	CompletedQuests.AddTag(InQuestTag);
	UpdateQuestStatus(InQuestTag, EQuestTreeStatus::Completed);

	if (bNotifyGraph)
		ExecuteQuestTrees();
}

void UQuestTreeSubsystem::FailQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (ExpiredQuests.HasTagExact(InQuestTag)
		&& !FailedQuests.HasTagExact(InQuestTag))
		return;

	AvailableQuests.RemoveTag(InQuestTag);
	ActiveQuests.RemoveTag(InQuestTag);
	FailedQuests.AddTag(InQuestTag);
	UpdateQuestStatus(InQuestTag, EQuestTreeStatus::Failed);

	if (bNotifyGraph)
		ExecuteQuestTrees();
}

void UQuestTreeSubsystem::ExpireQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (CompletedQuests.HasTagExact(InQuestTag) || ExpiredQuests.HasTagExact(InQuestTag))
		return;

	AvailableQuests.RemoveTag(InQuestTag);
	ActiveQuests.RemoveTag(InQuestTag);
	FailedQuests.RemoveTag(InQuestTag);
	ExpiredQuests.AddTag(InQuestTag);
	UpdateQuestStatus(InQuestTag, EQuestTreeStatus::Expired);

	if (bNotifyGraph)
		ExecuteQuestTrees();
}

EQuestTreeStatus UQuestTreeSubsystem::GetQuestStatus(const FGameplayTag& InQuestTag) const
{
	if (const FQuestTreeDataRecord* FoundQuestRecord = QuestDataRecords.Find(InQuestTag))
		return FoundQuestRecord->QuestStatus;

	return EQuestTreeStatus::None;
}

void UQuestTreeSubsystem::ExecuteQuestEvent(const FGameplayTag& InEventTag, bool bNotifyGraph)
{
	if (!ExecutedEvents.HasTagExact(InEventTag))
	{
		ExecutedEvents.AddTag(InEventTag);

		if (bNotifyGraph)
			ExecuteQuestTrees();
	}
}

bool UQuestTreeSubsystem::DoesActiveQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const
{
	return InQuestQuery.Matches(ActiveQuests);
}

bool UQuestTreeSubsystem::DoesCompletedQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const
{
	return InQuestQuery.Matches(CompletedQuests);
}

bool UQuestTreeSubsystem::DoesExpiredQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const
{
	return InQuestQuery.Matches(ExpiredQuests);
}

bool UQuestTreeSubsystem::DoesExecutedEventsMatchesQuery(const FGameplayTagQuery& InEventQuery) const
{
	return InEventQuery.Matches(ExecutedEvents);
}

bool UQuestTreeSubsystem::IsEventExecuted(const FGameplayTag& InEventTag) const
{
	return ExecutedEvents.HasTagExact(InEventTag);
}

bool UQuestTreeSubsystem::FindQuestDataFromQuestGraphsByTag(const FGameplayTag& InQuestTag, FQuestTreeData& FoundQuestData) const
{
	FoundQuestData = FQuestTreeData();
	if (!InQuestTag.IsValid() || QuestTreeGraphs.Num() == 0)
		return false;

	for (const UQuestTreeGraph* QuestTreeGraph : QuestTreeGraphs)
	{
		if (QuestTreeGraph->FindQuestDataByTag(InQuestTag, FoundQuestData))
		{
			return true;
		}
	}

	return false;
}

bool UQuestTreeSubsystem::FindQuestGiversByQuestData(const FQuestTreeData& InQuestData, TArray<UQuestTreeGiverComponent*>& OutQuestGivers)
{
	OutQuestGivers.Empty();
	for (UQuestTreeGiverComponent* QuestGiver : RegisteredQuestGivers)
	{
		if (IsValid(QuestGiver))
		{
			if (InQuestData.QuestGiverTag.HasTagExact(QuestGiver->GetQuestGiverTag()))
			{
				OutQuestGivers.AddUnique(QuestGiver);
			}
		}
	}

	return OutQuestGivers.Num() > 0;
}

bool UQuestTreeSubsystem::FindQuestGiversByTag(const FGameplayTagContainer& InQuestGiversTag, TArray<UQuestTreeGiverComponent*>& OutQuestGivers)
{
	OutQuestGivers.Empty();
	for (UQuestTreeGiverComponent* QuestGiver : RegisteredQuestGivers)
	{
		if (IsValid(QuestGiver))
		{
			if (InQuestGiversTag.HasTagExact(QuestGiver->GetQuestGiverTag()))
			{
				OutQuestGivers.AddUnique(QuestGiver);
			}
		}
	}

	return OutQuestGivers.Num() > 0;
}

void UQuestTreeSubsystem::ExecuteQuestTrees()
{
	/*if (!bQuestDatabaseLoaded)
		return;*/
	for (UQuestTreeGraph* QuestTree : QuestTreeGraphs)
	{
		QuestTree->ExecuteQuestTree(OwningQuestManager);
	}
}

bool UQuestTreeSubsystem::FindQuestsRecordByQuestTag(const FGameplayTag& InQuestTag, FQuestTreeDataRecord& OutQuestRecord) const
{
	if (const FQuestTreeDataRecord* FoundQuestData = QuestDataRecords.Find(InQuestTag))
	{
		OutQuestRecord = *FoundQuestData;
		return true;
	}

	return false;
}

TArray<FQuestTreeDataRecord> UQuestTreeSubsystem::GetAllQuestsRecords() const
{
	TArray<FQuestTreeDataRecord> FoundRecords;
	QuestDataRecords.GenerateValueArray(FoundRecords);
	return FoundRecords;
}

FQuestTreeDataRecord* UQuestTreeSubsystem::FindOrAddQuestDataRecordByRef(const FGameplayTag& InQuestTag)
{
	if (FQuestTreeDataRecord* FoundQuestData = QuestDataRecords.Find(InQuestTag))
	{
		return FoundQuestData;
	}
	else
	{
		FQuestTreeData InitialQuestData;
		if (ensureMsgf(FindQuestDataFromQuestGraphsByTag(InQuestTag, InitialQuestData), TEXT("UQuestTreeSubsystem:: Couldn't find quest data for quest tag [%s] from registered QuestTreeGraphs"), *InQuestTag.ToString()))
		{
			FQuestTreeDataRecord NewQuestRecord(InitialQuestData);
			InitialQuestData.Objectives.Empty();
			NewQuestRecord.QuestEvent = nullptr;
			QuestDataRecords.Add(InQuestTag, NewQuestRecord);
			return QuestDataRecords.Find(InQuestTag);
		}
	}

	return nullptr;
}

FQuestTreeDataRecord* UQuestTreeSubsystem::FindQuestDataRecordByRef(const FGameplayTag& InQuestTag)
{
	if (FQuestTreeDataRecord* FoundQuestData = QuestDataRecords.Find(InQuestTag))
	{
		return FoundQuestData;
	}

	return nullptr;
}

void UQuestTreeSubsystem::UpdateQuestStatus(const FGameplayTag& InQuestTag, const EQuestTreeStatus& NewStatus)
{
	FQuestTreeDataRecord* FoundQuestDataRecord = FindQuestDataRecordByRef(InQuestTag);
	if (!ensureMsgf(FoundQuestDataRecord, TEXT("UQuestTreeSubsystem::UpdateQuestStatus: Couldn't find quest record with tag [%s]"), *InQuestTag.ToString()))
		return;

	FoundQuestDataRecord->QuestStatus = NewStatus;
	
	// Notify quest givers for quest new status
	TArray<UQuestTreeGiverComponent*> FoundQuestGivers;
	if (FindQuestGiversByTag(FoundQuestDataRecord->QuestGiverTag, FoundQuestGivers))
	{
		for (UQuestTreeGiverComponent* QuestGiver : FoundQuestGivers)
			QuestGiver->OnQuestStatusUpdatedForQuestData(*FoundQuestDataRecord, NewStatus, OwningQuestManager);
	}
	
	// Activate quest first objective if quest just activated.
	if (NewStatus == EQuestTreeStatus::InProgress)
	{
		CreateObjectivesForQuestRecord(FoundQuestDataRecord);
		if (ensureMsgf(FoundQuestDataRecord->Objectives.Num() > 0, TEXT("Activated Quest [%s] has no objective assigned to it"), *InQuestTag.ToString()))
		{
			UQuestTreeObjective* FirstObjective = FoundQuestDataRecord->Objectives[0];
			if (ensure(FirstObjective))
			{
				FirstObjective->OnObjectiveFinished.AddUniqueDynamic(this, &UQuestTreeSubsystem::OnQuestObjectiveFinished);
				FirstObjective->ActivateObjective(OwningQuestManager, *FoundQuestDataRecord);
				FoundQuestDataRecord->ObjectivesStatus[FirstObjective->ObjectiveTag] = FirstObjective->GetObjectiveStatus();
			}
		}

		if (IsValid(FoundQuestDataRecord->QuestEvent))
		{
			FoundQuestDataRecord->QuestEvent->OnActivateEvent(OwningQuestManager, this, *FoundQuestDataRecord);
		}
	}
	else
	{
		if (IsValid(FoundQuestDataRecord->QuestEvent))
		{
			// Deactivate event if current quest was not active any longer.
			FoundQuestDataRecord->QuestEvent->OnDeactivateEvent(OwningQuestManager, this, *FoundQuestDataRecord);
			FoundQuestDataRecord->QuestEvent->ConditionalBeginDestroy();
			FoundQuestDataRecord->QuestEvent->MarkAsGarbage();
			FoundQuestDataRecord->QuestEvent = nullptr;
		}
	}

	BroadcastAnyQuestUpdate(FoundQuestDataRecord);
}

void UQuestTreeSubsystem::UpdateRelaventQuestRecordsForQuestGiver(UQuestTreeGiverComponent* InQuestGiver)
{
	for (TPair<FGameplayTag, FQuestTreeDataRecord> QuestData : QuestDataRecords)
	{
		if (QuestData.Value.QuestGiverTag.HasTagExact(InQuestGiver->GetQuestGiverTag()))
		{
			InQuestGiver->RelaventQuestRecords.Add(QuestData);
		}
	}
}

void UQuestTreeSubsystem::UpdateRegisteredQuestGiverStatus(UQuestTreeGiverComponent* InQuestGiver)
{
	if (!IsValid(OwningQuestManager))
		return;

	for (const TPair<FGameplayTag, FQuestTreeDataRecord>& QuestData : QuestDataRecords)
	{
		if (QuestData.Value.QuestGiverTag.HasTagExact(InQuestGiver->GetQuestGiverTag()))
		{
			InQuestGiver->OnQuestStatusUpdatedForQuestData(QuestData.Value, QuestData.Value.QuestStatus, OwningQuestManager);
		}
	}
}

void UQuestTreeSubsystem::CreateObjectivesForQuestRecord(FQuestTreeDataRecord* InQuestRecord)
{
	// Do not create objectives if already created
	if (InQuestRecord->Objectives.Num())
		return;

	const FGameplayTag QuestTag = InQuestRecord->QuestTag;
	FQuestTreeData InitialQuestData;
	if (ensureMsgf(FindQuestDataFromQuestGraphsByTag(QuestTag, InitialQuestData), TEXT("UQuestTreeSubsystem::CreateObjectivesForQuestRecord: Couldn't find quest data for quest tag [%s] from registered QuestTreeGraphs"), *QuestTag.ToString()))
	{
		for (UQuestTreeObjective* Objective : InitialQuestData.Objectives)
		{
			if (!IsValid(Objective))
				continue;
			
			const FGameplayTag& ObjectiveTag(Objective->ObjectiveTag);
			UQuestTreeObjective* NewObjective = DuplicateObject<UQuestTreeObjective>(Objective, this);
			InQuestRecord->Objectives.AddUnique(NewObjective);
			
			// Do no add objective status if it already exist, if it already exist it means the QuestDataRecord is from loaded game.
			if (!InQuestRecord->ObjectivesStatus.Contains(ObjectiveTag))
				InQuestRecord->ObjectivesStatus.Add(ObjectiveTag, EQuestTreeObjectiveStatus::None);
			
			if (IsValid(InitialQuestData.Event))
			{
				UQuestTreeActivatableEvent* NewObjectiveEvent = DuplicateObject<UQuestTreeActivatableEvent>(InitialQuestData.Event, this);
				InQuestRecord->QuestEvent = NewObjectiveEvent;
			}
		}
	}
}

void UQuestTreeSubsystem::LoadObjectivesRecordsForQuestRecord(FQuestTreeDataRecord* InQuestRecord)
{
	for (UQuestTreeObjective* Objective : InQuestRecord->Objectives)
	{
		const FGameplayTag& ObjectiveTag(Objective->ObjectiveTag);
		if (!InQuestRecord->ObjectivesStatus.Contains(ObjectiveTag))
			continue;

		Objective->ObjectiveStatus = InQuestRecord->ObjectivesStatus[ObjectiveTag];
		if (Objective->ObjectiveStatus == EQuestTreeObjectiveStatus::InProgress)
		{
			// Load in progress objective records.
			if (InQuestRecord->ObjectivesJsonData.Contains(ObjectiveTag))
				Objective->LoadObjectiveRecordsFromJson(InQuestRecord->ObjectivesJsonData[ObjectiveTag]);

			// Load in progress objective event records.
			if (InQuestRecord->ObjectivesEventJsonData.Contains(ObjectiveTag))
				Objective->LoadEventRecordsFromJson(InQuestRecord->ObjectivesEventJsonData[ObjectiveTag]);
		}
	}

	if (IsValid(InQuestRecord->QuestEvent))
	{
		InQuestRecord->QuestEvent->LoadEventRecordsFromJson(InQuestRecord->QuestEventJsonData);
	}
}

void UQuestTreeSubsystem::ResumeActiveQuestsObjectives()
{
	for (const FGameplayTag& ActiveQuest : ActiveQuests)
	{
		if (FQuestTreeDataRecord* QuestRecord = FindQuestDataRecordByRef(ActiveQuest))
		{
			if (!ensure(QuestRecord->QuestStatus == EQuestTreeStatus::InProgress))
				continue;

			CreateObjectivesForQuestRecord(QuestRecord);
			LoadObjectivesRecordsForQuestRecord(QuestRecord);
			for (UQuestTreeObjective* Objective : QuestRecord->Objectives)
			{
				if (Objective && Objective->GetObjectiveStatus() == EQuestTreeObjectiveStatus::InProgress)
				{
					Objective->OnObjectiveFinished.AddUniqueDynamic(this, &UQuestTreeSubsystem::OnQuestObjectiveFinished);
					Objective->ResumeObjective(OwningQuestManager, *QuestRecord);
				}
			}

			if (IsValid(QuestRecord->QuestEvent))
				QuestRecord->QuestEvent->OnResumeEvent(OwningQuestManager, this, *QuestRecord);
		}
	}
}

void UQuestTreeSubsystem::BroadcastAnyQuestUpdate(FQuestTreeDataRecord* InQuestRecord)
{
	OnAnyQuestUpdated.Broadcast(*InQuestRecord, InQuestRecord->QuestStatus);

	TWeakObjectPtr<UQuestTreeSubsystem> WeakThis = this;
	GetWorld()->GetTimerManager().ClearTimer(CleanupQuestObjectivesTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(CleanupQuestObjectivesTimerHandle, [WeakThis]
		{
			for (TPair<FGameplayTag, FQuestTreeDataRecord>& QuestRecord : WeakThis->QuestDataRecords)
			{
				if (QuestRecord.Value.QuestStatus == EQuestTreeStatus::InProgress)
					continue;

				// Destroy all objectives if quest is not in progress any longer.
				for (UQuestTreeObjective* Objective : QuestRecord.Value.Objectives)
				{
					if (!Objective)
						continue;

					Objective->ConditionalBeginDestroy();
					Objective->MarkAsGarbage();
					Objective = nullptr;
				}

				QuestRecord.Value.Objectives.Empty();
			}
		}, 0.2f, false);
}

void UQuestTreeSubsystem::OnQuestObjectiveFinished(bool bWasSuccess, const FGameplayTag& QuestTag, UQuestTreeObjective* FinishedObjective)
{
	if (!ensure(IsValid(FinishedObjective)))
		return;

	FinishedObjective->OnObjectiveFinished.RemoveDynamic(this, &UQuestTreeSubsystem::OnQuestObjectiveFinished);

	FQuestTreeDataRecord* FoundQuestRecord = FindQuestDataRecordByRef(QuestTag);
	if (!ensureMsgf(FoundQuestRecord, TEXT("UQuestTreeSubsystem::UpdateQuestStatus: Couldn't find quest record with tag [%s]"), *QuestTag.ToString()))
		return;

	// Fail quest if finished unsuccessful
	if (!bWasSuccess && FinishedObjective->DoesFailQuestIfUnsuccessful())
	{
		FailQuest(QuestTag);
	}
	else
	{
		int32 FinishedObjectiveIndex = -1;
		if (!ensureMsgf(FoundQuestRecord->Objectives.Find(FinishedObjective, FinishedObjectiveIndex), TEXT("UQuestTreeSubsystem::UpdateQuestStatus: Couldn't find finished objective index")))
			return;

		// Find next objective
		int32 NextObjectiveIndex = FinishedObjectiveIndex + 1;
		if (FoundQuestRecord->Objectives.IsValidIndex(NextObjectiveIndex))
		{
			// Activate next objective if available
			UQuestTreeObjective* NextObjective = FoundQuestRecord->Objectives[NextObjectiveIndex];
			if (ensureMsgf(NextObjective, TEXT("Quest [%s] has invalid objective at index [%d]"), *QuestTag.ToString(), NextObjectiveIndex))
			{
				NextObjective->OnObjectiveFinished.AddUniqueDynamic(this, &UQuestTreeSubsystem::OnQuestObjectiveFinished);
				NextObjective->ActivateObjective(OwningQuestManager, *FoundQuestRecord);
			}
		}
		else
		{
			if (bWasSuccess)
			{
				// Complete quest since it was last objective.
				CompleteQuest(QuestTag);
			}
			else
			{
				// Fail the quest even if "bFailQuestIfUnsuccessful" was false since there is no next objective to proceed to.
				FailQuest(QuestTag);
			}
		}
	}

	// Sync objective status to the QuestDataRecords.
	for (UQuestTreeObjective* Objective : FoundQuestRecord->Objectives)
	{
		if (!Objective)
			continue;

		if (ensure(FoundQuestRecord->ObjectivesStatus.Contains(Objective->ObjectiveTag)))
			FoundQuestRecord->ObjectivesStatus[Objective->ObjectiveTag] = Objective->GetObjectiveStatus();
	}

	BroadcastAnyQuestUpdate(FoundQuestRecord);
}

void UQuestTreeSubsystem::OnRequestSaveDataInSlot(const FString& InSlotName)
{
	UProfileManagerSubsystem* ProfileManager = UProfileManagerSubsystem::Get(this);
	FSaveSlotInfo* SaveInfo = ProfileManager->FindOrAddSaveSlotForActiveProfile(InSlotName);
	
	SaveLocalQuestDatabaseInSlot(SaveInfo->QuestSlotName);
	ProfileManager->InformSystemSaveDone(this);
}

void UQuestTreeSubsystem::OnRequestLoadDataInSlot(const FString& InSlotName)
{
	UProfileManagerSubsystem* ProfileManager = UProfileManagerSubsystem::Get(this);
	FSaveSlotInfo SaveInfo;

	if (ProfileManager->FindSaveSlotForActiveProfile(InSlotName, SaveInfo))
		LoadLocalQuestDatabaseForProfile(SaveInfo.QuestSlotName);

	ProfileManager->InformSystemLoadDone(this);
}

void UQuestTreeSubsystem::StartedLoadingLevel(const FString& InLevelName)
{
	for (const FGameplayTag& ActiveQuest : ActiveQuests)
	{
		if (FQuestTreeDataRecord* QuestRecord = FindQuestDataRecordByRef(ActiveQuest))
		{
			if (!ensure(QuestRecord->QuestStatus == EQuestTreeStatus::InProgress))
				continue;

			if (IsValid(QuestRecord->QuestEvent))
				QuestRecord->QuestEvent->PauseEvent();
		}
	}
}

void UQuestTreeSubsystem::FinishedLoadingLevel(const FString& InLevelName)
{
	for (const FGameplayTag& ActiveQuest : ActiveQuests)
	{
		if (FQuestTreeDataRecord* QuestRecord = FindQuestDataRecordByRef(ActiveQuest))
		{
			if (!ensure(QuestRecord->QuestStatus == EQuestTreeStatus::InProgress))
				continue;

			if (IsValid(QuestRecord->QuestEvent))
				QuestRecord->QuestEvent->UnpauseEvent();
		}
	}
}