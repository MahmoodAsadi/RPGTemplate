// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/QuestTreeEvents.h"

#include "Engine/World.h"
#include "JsonObjectConverter.h"


void UQuestTreeActivatableEvent::Tick(float DeltaTime)
{
	K2_Tick(DeltaTime);
}

bool UQuestTreeActivatableEvent::IsTickable() const
{
	if (!bCanTick)
		return false;
	
	UWorld* World = GetWorld();
	if (!IsValid(World))
		return false;

	return !bIsPaused;
}

void UQuestTreeActivatableEvent::OnActivateEvent(UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord)
{
	K2_OnActivateEvent(InQuestManager, EventOwner, InQuestRecord);
}

void UQuestTreeActivatableEvent::OnResumeEvent(UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord)
{
	K2_OnResumeEvent(InQuestManager, EventOwner, InQuestRecord);
}

void UQuestTreeActivatableEvent::OnDeactivateEvent(UQuestTreeManagerComponent* InQuestManager, UObject* EventOwner, const FQuestTreeDataRecord& InQuestRecord)
{
	bCanTick = false;
	K2_OnDeactivateEvent(InQuestManager, EventOwner, InQuestRecord);
}

void UQuestTreeActivatableEvent::PauseEvent()
{
	bIsPaused = true;
	K2_PauseEvent();
}

void UQuestTreeActivatableEvent::UnpauseEvent()
{
	bIsPaused = false;
	K2_UnpauseEvent();
}

void UQuestTreeActivatableEvent::PrepareEventForSave(UQuestTreeManagerComponent* InQuestManager)
{
	K2_PrepareEventForSave(InQuestManager);
}

FString UQuestTreeActivatableEvent::GetEventRecordsAsJason(UQuestTreeManagerComponent* InQuestManager)
{
	PrepareEventForSave(InQuestManager);

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

void UQuestTreeActivatableEvent::LoadEventRecordsFromJson(const FString& InJsonString)
{
	if (InJsonString.IsEmpty())
		return;

	// Find Records property.
	FProperty* RecordsProperty = GetClass()->FindPropertyByName("Records");
	if (!RecordsProperty)
		return;

	FStructProperty* StructProperty = CastField<FStructProperty>(RecordsProperty);
	if (!ensureMsgf(StructProperty, TEXT("UQuestTreeActivatableEvent::LoadEventRecordsFromJson: Variable 'Records' is not an struct type.")))
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

void UQuestTreeExecutableEvent::ExecuteEvent(UQuestTreeManagerComponent* InQuestManager) const
{
	K2_ExecuteEvent(InQuestManager);
}