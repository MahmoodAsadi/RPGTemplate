// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/QuestTreeHelper.h"
#include "HAL/PlatformProperties.h"
#include "Components/Widget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(QuestTreeHelper)

FQuestTreeDataRecord::FQuestTreeDataRecord(const FQuestTreeData& InQuestData)
{
	QuestTag = InQuestData.QuestTag;
	QuestGiverTag = InQuestData.QuestGiverTag;
}

UQuestTreeHelper::UQuestTreeHelper(const FObjectInitializer& ObjectInitializer)
{
	
}

void UQuestTreeHelper::ClearAndInvalidateTimerByHandle(const UObject* Object, FTimerHandle& Handle)
{
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(Handle);
		}
	}
}

bool UQuestTreeHelper::DoesTimerExistsByHandle(const UObject* Object, FTimerHandle Handle)
{
	bool bTimerExists = false;
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			bTimerExists = World->GetTimerManager().TimerExists(Handle);
		}
	}

	return bTimerExists;
}

float UQuestTreeHelper::GetTimerElapsedTimeByHandle(const UObject* Object, FTimerHandle Handle)
{
	float ElapsedTime = 0.0f;
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			ElapsedTime = World->GetTimerManager().GetTimerElapsed(Handle);
		}
	}

	return ElapsedTime;
}

float UQuestTreeHelper::GetTimerRemainingTimeByHandle(const UObject* Object, FTimerHandle Handle)
{
	float RemainingTime = 0.0f;
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			RemainingTime = World->GetTimerManager().GetTimerRemaining(Handle);
		}
	}

	return RemainingTime;
}

bool UQuestTreeHelper::IsTimerActiveByHandle(const UObject* Object, FTimerHandle Handle)
{
	bool bIsActive = false;
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			bIsActive = World->GetTimerManager().IsTimerActive(Handle);
		}
	}

	return bIsActive;
}

bool UQuestTreeHelper::IsTimerPausedByHandle(const UObject* Object, FTimerHandle Handle)
{
	bool bIsPaused = false;
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			bIsPaused = World->GetTimerManager().IsTimerPaused(Handle);
		}
	}

	return bIsPaused;
}

void UQuestTreeHelper::PauseTimerByHandle(const UObject* Object, FTimerHandle Handle)
{
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			World->GetTimerManager().PauseTimer(Handle);
		}
	}
}

void UQuestTreeHelper::UnPauseTimerByHandle(const UObject* Object, FTimerHandle Handle)
{
	if (Handle.IsValid())
	{
		UWorld* World = Object->GetWorld();
		if (World)
		{
			World->GetTimerManager().UnPauseTimer(Handle);
		}
	}
}