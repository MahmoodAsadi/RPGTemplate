// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/QuestTreeObjectives.h"

#include "Engine/World.h"
#include "JsonObjectConverter.h"

#include "Components/QuestTreeManagerComponent.h"
#include "Core/QuestTreeSubsystem.h"
#include "Core/QuestTreeEvents.h"
#include "ProfileManagerSubsystem.h"


FTimeBasedObjectiveRecord::FTimeBasedObjectiveRecord(const FString& InJasonString)
{
	FJsonObjectConverter::JsonObjectStringToUStruct(InJasonString, this);
}

void UQuestTreeObjective::Tick(float DeltaTime)
{
	K2_Tick(DeltaTime);
}

bool UQuestTreeObjective::IsTickable() const
{
	if (!bCanTick)
		return false;

	UWorld* World = GetWorld();
	if (!IsValid(World))
		return false;

	return ObjectiveStatus == EQuestTreeObjectiveStatus::InProgress && !bIsPaused;
}

void UQuestTreeObjective::ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	OwningQuestManager = ForQuestManager;
	QuestRecord = InQuestRecord;
	OwningQuestTag = QuestRecord.QuestTag;
	ObjectiveStatus = EQuestTreeObjectiveStatus::InProgress;
	UProfileManagerSubsystem::Get(this)->OnStartLoadingLevel.AddUniqueDynamic(this, &UQuestTreeObjective::StartedLoadingLevel);
	UProfileManagerSubsystem::Get(this)->OnLoadingLevelFinished.AddUniqueDynamic(this, &UQuestTreeObjective::FinishedLoadingLevel);

	if (IsValid(ObjectiveEvent))
		ObjectiveEvent->OnActivateEvent(ForQuestManager, this, InQuestRecord);
}

void UQuestTreeObjective::ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	OwningQuestManager = ForQuestManager;
	QuestRecord = InQuestRecord;
	OwningQuestTag = QuestRecord.QuestTag;
	ObjectiveStatus = EQuestTreeObjectiveStatus::InProgress;
	UProfileManagerSubsystem::Get(this)->OnStartLoadingLevel.AddUniqueDynamic(this, &UQuestTreeObjective::StartedLoadingLevel);
	UProfileManagerSubsystem::Get(this)->OnLoadingLevelFinished.AddUniqueDynamic(this, &UQuestTreeObjective::FinishedLoadingLevel);
	
	if (IsValid(ObjectiveEvent))
		ObjectiveEvent->OnResumeEvent(OwningQuestManager, this, QuestRecord);
}

void UQuestTreeObjective::PauseObjective()
{
	bIsPaused = true;
	if (IsValid(ObjectiveEvent))
		ObjectiveEvent->PauseEvent();
}

void UQuestTreeObjective::UnPauseObjective()
{
	bIsPaused = false;
	if (IsValid(ObjectiveEvent))
		ObjectiveEvent->UnpauseEvent();
}

void UQuestTreeObjective::FinishedObjective(bool bWasSuccess)
{
	ObjectiveStatus = bWasSuccess ? EQuestTreeObjectiveStatus::Success : EQuestTreeObjectiveStatus::Failed;
	OnObjectiveFinished.Broadcast(bWasSuccess, OwningQuestTag, this);
	UProfileManagerSubsystem::Get(this)->OnStartLoadingLevel.RemoveDynamic(this, &UQuestTreeObjective::StartedLoadingLevel);
	UProfileManagerSubsystem::Get(this)->OnLoadingLevelFinished.RemoveDynamic(this, &UQuestTreeObjective::FinishedLoadingLevel);
	
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearAllTimersForObject(this);

	if (IsValid(ObjectiveEvent))
	{
		ObjectiveEvent->OnDeactivateEvent(OwningQuestManager, this, QuestRecord);
		ObjectiveEvent->ConditionalBeginDestroy();
		ObjectiveEvent->MarkAsGarbage();
		ObjectiveEvent = nullptr;
	}
}

void UQuestTreeObjective::FinishObjective(bool bSuccess)
{
	FinishedObjective(bSuccess);
}

FString UQuestTreeObjective::GetObjectiveRecordsAsJson(UQuestTreeManagerComponent* InQuestManager)
{
	PrepareObjectiveForSave(InQuestManager);

	FString OutString;
	// Find Records property.
	FProperty* RecordsProperty = GetClass()->FindPropertyByName("Records");
	if (!RecordsProperty)
		return OutString;
	
	FStructProperty* StructProperty = CastField<FStructProperty>(RecordsProperty);
	if (!StructProperty)
		return OutString;

	uint8* StructPropData = StructProperty->ContainerPtrToValuePtr<uint8>(this);
	FJsonObjectConverter::UStructToJsonObjectString(StructProperty->Struct, StructPropData, OutString);
	return OutString;
}

FString UQuestTreeObjective::GetEventRecordsAsJason(UQuestTreeManagerComponent* InQuestManager)
{
	if (!IsValid(ObjectiveEvent))
		return FString();

	return ObjectiveEvent->GetEventRecordsAsJason(InQuestManager);
}

void UQuestTreeObjective::LoadObjectiveRecordsFromJson(const FString& InJsonString)
{
	// Find Records property.
	FProperty* RecordsProperty = GetClass()->FindPropertyByName("Records");
	if (!RecordsProperty)
		return;

	FStructProperty* StructProperty = CastField<FStructProperty>(RecordsProperty);
	if (!ensureMsgf(StructProperty, TEXT("UQuestTreeObjective::LoadObjectiveRecordsFromJson: Variable 'Records' is not an struct type.")))
		return;
	
	TSharedPtr<FJsonValue> jsonParsed;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(InJsonString);
	if (FJsonSerializer::Deserialize(Reader, jsonParsed))
	{
		TSharedPtr<FJsonObject> jsonObject = jsonParsed.Get()->AsObject();
		if (jsonObject != nullptr)
		{
			uint8* StructPropData = StructProperty->ContainerPtrToValuePtr<uint8>(this);
			FJsonObjectConverter::JsonObjectToUStruct(jsonObject.ToSharedRef(), StructProperty->Struct, StructPropData);
		}
	}
}

void UQuestTreeObjective::LoadEventRecordsFromJson(const FString& InJsonString)
{
	if (!IsValid(ObjectiveEvent))
		return;

	ObjectiveEvent->LoadEventRecordsFromJson(InJsonString);
}

void UQuestTreeObjective::PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager)
{
	if (!IsValid(InQuestManager))
		return;

	if (IsValid(ObjectiveEvent))
		ObjectiveEvent->PrepareEventForSave(InQuestManager);
}

void UQuestTreeObjective::StartedLoadingLevel(const FString& InLevelName)
{
	PauseObjective();
}

void UQuestTreeObjective::FinishedLoadingLevel(const FString& InLevelName)
{
	UnPauseObjective();
}

void UQuestTreeObjective_Wait::ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	Super::ActivateObjective(ForQuestManager, InQuestRecord);

	if (!IsValid(ForQuestManager))
		return;
	
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(ObjectiveTimerHandle))
		return;

	Duration = FMath::RoundToInt32(Duration);
	
	TWeakObjectPtr<UQuestTreeObjective_Wait> WeakThis = this;
	TimerManager.SetTimer(ObjectiveTimerHandle, [WeakThis]
		{
			FTimerManager& TimerManager = WeakThis->GetWorld()->GetTimerManager();
			TimerManager.ClearTimer(WeakThis->ObjectiveTimerHandle);
			WeakThis.Get()->FinishObjective(true);
		}, Duration, false);
}

void UQuestTreeObjective_Wait::ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	Super::ResumeObjective(ForQuestManager, InQuestRecord);

	if (!IsValid(ForQuestManager))
		return;

	FTimerManager& TimerManager = ForQuestManager->GetWorld()->GetTimerManager();
	if (TimerManager.TimerExists(ObjectiveTimerHandle))
		return;
	
	Records.RemainingTime = FMath::RoundToInt32(Records.RemainingTime);
	
	TWeakObjectPtr<UQuestTreeObjective_Wait> WeakThis = this;
	TimerManager.SetTimer(ObjectiveTimerHandle, [WeakThis]
		{
			FTimerManager& TimerManager = WeakThis->GetWorld()->GetTimerManager();
			TimerManager.ClearTimer(WeakThis->ObjectiveTimerHandle);
			WeakThis.Get()->FinishObjective(true);
		}, Records.RemainingTime, false);
}

void UQuestTreeObjective_Wait::PauseObjective()
{
	Super::PauseObjective();
	
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(ObjectiveTimerHandle))
		TimerManager.PauseTimer(ObjectiveTimerHandle);
}

void UQuestTreeObjective_Wait::UnPauseObjective()
{
	Super::UnPauseObjective();

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.TimerExists(ObjectiveTimerHandle) && TimerManager.IsTimerPaused(ObjectiveTimerHandle))
		TimerManager.UnPauseTimer(ObjectiveTimerHandle);
}

void UQuestTreeObjective_Wait::PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager)
{
	Super::PrepareObjectiveForSave(InQuestManager);

	if (!IsValid(InQuestManager))
		return;

	FTimerManager& TimerManager = InQuestManager->GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(ObjectiveTimerHandle))
		Records.RemainingTime = TimerManager.GetTimerRemaining(ObjectiveTimerHandle);
}

float UQuestTreeObjective_Wait::GetRemainingTime_Implementation()
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(ObjectiveTimerHandle);
}

void UQuestTreeObjective_CountDown::ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	Super::ActivateObjective(ForQuestManager, InQuestRecord);

	if (!IsValid(ForQuestManager))
		return;

	FTimerManager& TimerManager = ForQuestManager->GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(ObjectiveTimerHandle))
		return;

	Duration = FMath::RoundToInt32(Duration);
	TWeakObjectPtr<UQuestTreeObjective_CountDown> WeakThis = this;
	TimerManager.SetTimer(ObjectiveTimerHandle, [WeakThis]
		{
			FTimerManager& TimerManager = WeakThis->GetWorld()->GetTimerManager();
			TimerManager.ClearTimer(WeakThis->ObjectiveTimerHandle);
			WeakThis.Get()->FinishObjective(false);
		}, Duration, false);
}

void UQuestTreeObjective_CountDown::ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	Super::ResumeObjective(ForQuestManager, InQuestRecord);

	if (!IsValid(ForQuestManager))
		return;

	FTimerManager& TimerManager = ForQuestManager->GetWorld()->GetTimerManager();
	if (TimerManager.TimerExists(ObjectiveTimerHandle))
		return;
	
	Records.RemainingTime = FMath::RoundToInt32(Records.RemainingTime);
	TWeakObjectPtr<UQuestTreeObjective_CountDown> WeakThis = this;
	TimerManager.SetTimer(ObjectiveTimerHandle, [WeakThis]
		{
			FTimerManager& TimerManager = WeakThis->GetWorld()->GetTimerManager();
			TimerManager.ClearTimer(WeakThis->ObjectiveTimerHandle);
			WeakThis.Get()->FinishObjective(false);
		}, Records.RemainingTime, false);
}

void UQuestTreeObjective_CountDown::PauseObjective()
{
	Super::PauseObjective();

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(ObjectiveTimerHandle))
		TimerManager.PauseTimer(ObjectiveTimerHandle);
}

void UQuestTreeObjective_CountDown::UnPauseObjective()
{
	Super::UnPauseObjective();

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.TimerExists(ObjectiveTimerHandle) && TimerManager.IsTimerPaused(ObjectiveTimerHandle))
		TimerManager.UnPauseTimer(ObjectiveTimerHandle);
}

void UQuestTreeObjective_CountDown::PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager)
{
	Super::PrepareObjectiveForSave(InQuestManager);

	if (!IsValid(InQuestManager))
		return;

	FTimerManager& TimerManager = InQuestManager->GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(ObjectiveTimerHandle))
		Records.RemainingTime = TimerManager.GetTimerRemaining(ObjectiveTimerHandle);
}

float UQuestTreeObjective_CountDown::GetRemainingTime_Implementation()
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(ObjectiveTimerHandle);
}

void UQuestTreeObjective_Custom::ActivateObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	Super::ActivateObjective(ForQuestManager, InQuestRecord);
	K2_ActivateObjective(ForQuestManager, InQuestRecord);
}

void UQuestTreeObjective_Custom::ResumeObjective(UQuestTreeManagerComponent* ForQuestManager, const FQuestTreeDataRecord& InQuestRecord)
{
	Super::ResumeObjective(ForQuestManager, InQuestRecord);
	K2_ResumeObjective(ForQuestManager, InQuestRecord);
}

void UQuestTreeObjective_Custom::FinishedObjective(bool bWasSuccess)
{
	Super::FinishedObjective(bWasSuccess);
	K2_FinishedObjective(bWasSuccess);
}

void UQuestTreeObjective_Custom::PauseObjective()
{
	Super::PauseObjective();
	K2_PauseObjective();
}

void UQuestTreeObjective_Custom::UnPauseObjective()
{
	Super::UnPauseObjective();
	K2_UnPauseObjective();
}

void UQuestTreeObjective_Custom::PrepareObjectiveForSave(UQuestTreeManagerComponent* InQuestManager)
{
	Super::PrepareObjectiveForSave(InQuestManager);
	K2_PrepareObjectiveForSave(InQuestManager);
}