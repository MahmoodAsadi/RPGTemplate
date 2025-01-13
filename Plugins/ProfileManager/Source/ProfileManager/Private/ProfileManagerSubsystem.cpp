// Fill out your copyright notice in the Description page of Project Settings.

#include "ProfileManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

#include "ProfilesSaveGame.h"


FString UProfileManagerSubsystem::ProfileSaveSlot = FString("UserProfile");

UProfileManagerSubsystem* UProfileManagerSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
		return nullptr;

	return World->GetGameInstance()->GetSubsystem<UProfileManagerSubsystem>();
}

bool UProfileManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void UProfileManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadProfileData();

	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &UProfileManagerSubsystem::StartedLoadingLevel);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UProfileManagerSubsystem::FinishedLoadingLevel);
}

void UProfileManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

}

bool UProfileManagerSubsystem::CreateProfile(const FString& InProfileName)
{
	if (InProfileName.IsEmpty() || ProfilesName.Contains(InProfileName))
		return false;

	ProfilesName.Add(InProfileName);
	ProfileSaveObject->Profiles = ProfilesName;

	if (ProfilesName.Num() == 1)
	{
		ActiveProfile = InProfileName;
		ProfileSaveObject->ActiveProfile = ActiveProfile;
	}

	return true;
}

bool UProfileManagerSubsystem::RemoveProfile(const FString& InProfileName)
{
	if (InProfileName.IsEmpty() || !ProfilesName.Contains(InProfileName))
		return false;

	ProfilesName.Remove(InProfileName);
	ProfilesSaveSlotsContainer.Remove(InProfileName);
	if (ActiveProfile == InProfileName)
		ActiveProfile = "";

	ProfileSaveObject->Profiles = ProfilesName;
	ProfileSaveObject->ActiveProfile = ActiveProfile;
	ProfileSaveObject->ProfilesSaveSlotsContainer = ProfilesSaveSlotsContainer;
	return true;
}

bool UProfileManagerSubsystem::ActivateProfile(const FString& InProfileName)
{
	if (InProfileName.IsEmpty() || !ProfilesName.Contains(InProfileName))
		return false;

	ActiveProfile = InProfileName;
	ProfileSaveObject->ActiveProfile = ActiveProfile;
	return true;
}

FSaveSlotInfo* UProfileManagerSubsystem::FindOrAddSaveSlotForActiveProfile(const FString& InSlotName)
{
	if (ProfilesSaveSlotsContainer.Contains(ActiveProfile))
	{
		if (ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Contains(InSlotName))
		{
			return ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Find(InSlotName);
		}
		
		ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Add(InSlotName, FSaveSlotInfo(InSlotName));
		return ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Find(InSlotName);
	}
	else
	{
		FProfileSaveSlots NewSaveSlot;
		NewSaveSlot.Slots.Add(InSlotName, FSaveSlotInfo(InSlotName));
		ProfilesSaveSlotsContainer.Add(TPair<FString, FProfileSaveSlots>(ActiveProfile, NewSaveSlot));
		return ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Find(InSlotName);
	}
}

bool UProfileManagerSubsystem::FindSaveSlotForActiveProfile(const FString& InSlotName, FSaveSlotInfo& OutSlotSaveData)
{
	if (ProfilesSaveSlotsContainer.Contains(ActiveProfile))
	{
		if (ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Contains(InSlotName))
		{
			OutSlotSaveData = *ProfilesSaveSlotsContainer.Find(ActiveProfile)->Slots.Find(InSlotName);
			return true;
		}
	}

	return false;
}

bool UProfileManagerSubsystem::RemoveSaveSlotFromActiveProfile(const FString& InSlotName)
{
	if (ActiveProfile.IsEmpty())
		return false;

	if (ProfilesSaveSlotsContainer.Contains(ActiveProfile) && ProfilesSaveSlotsContainer[ActiveProfile].Slots.Contains(InSlotName))
	{
		FSaveSlotInfo DeletingSaveSlot = ProfilesSaveSlotsContainer[ActiveProfile].Slots[InSlotName];

		if (UGameplayStatics::DoesSaveGameExist(DeletingSaveSlot.QuestSlotName, 0))
			UGameplayStatics::DeleteGameInSlot(DeletingSaveSlot.QuestSlotName, 0);

		if (UGameplayStatics::DoesSaveGameExist(DeletingSaveSlot.InventorySlotName, 0))
			UGameplayStatics::DeleteGameInSlot(DeletingSaveSlot.InventorySlotName, 0);
		
		ProfilesSaveSlotsContainer[ActiveProfile].Slots.Remove(InSlotName);
		ProfileSaveObject->ProfilesSaveSlotsContainer = ProfilesSaveSlotsContainer;
		SaveProfileData();
		return true;
	}
	
	return false;
}

TMap<FString, FSaveSlotInfo> UProfileManagerSubsystem::GetAllSaveDataForActiveProfile() const
{
	if (ProfilesSaveSlotsContainer.Contains(ActiveProfile))
	{
		return ProfilesSaveSlotsContainer[ActiveProfile].Slots;
	}

	return TMap<FString, FSaveSlotInfo>();
}

bool UProfileManagerSubsystem::DoesSaveSlotExist(const FString& InSlotName) const
{
	if (!ProfilesSaveSlotsContainer.Contains(ActiveProfile) || ProfilesSaveSlotsContainer[ActiveProfile].Slots.Num() == 0)
		return false;
	
	return ProfilesSaveSlotsContainer[ActiveProfile].Slots.Contains(InSlotName);
}

void UProfileManagerSubsystem::RequestSaveGameDataInSlot(const FString& InSlotName, const ESlotSaveType& InSaveType, FOnProfileActionFinished Event)
{
	if (bProfileActionInProgress)
		return;
	
	bProfileActionInProgress = true;
	SavingObjects = OnRequestSaveProfile.GetAllObjects();
	CurrentSavingSlotName = InSlotName;
	FSaveSlotInfo* SavingSlotInfo = FindOrAddSaveSlotForActiveProfile(CurrentSavingSlotName);
	SavingSlotInfo->SaveType = InSaveType;
	SavingSlotInfo->ActiveLevelName = ActiveLevelName;
	SavingSlotInfo->PlayerLastTransform = UGameplayStatics::GetPlayerPawn(this, 0)->GetActorTransform();
	
	if (SavingSlotInfo->InventorySlotName.IsEmpty())
		SavingSlotInfo->InventorySlotName = FGuid::NewGuid().ToString();

	if (SavingSlotInfo->QuestSlotName.IsEmpty())
		SavingSlotInfo->QuestSlotName = FGuid::NewGuid().ToString();

	ActiveActionDelegate = Event;
	OnRequestSaveProfile.Broadcast(InSlotName);
}

void UProfileManagerSubsystem::RequestLoadGameDataFromSlot(const FString& InSlotName, FOnProfileActionFinished Event)
{
	if (bProfileActionInProgress)
		return;
	
	bProfileActionInProgress = true;
	LoadingObjects = OnRequestLoadProfile.GetAllObjects();
	FSaveSlotInfo FoundInfo;
	if (FindSaveSlotForActiveProfile(InSlotName, FoundInfo))
		OpenLevelSpawnSettings = FOpenLevelPlayerSpawnSetting(EOpenLevelPlayerTransformType::FromLoad, FoundInfo.PlayerLastTransform);

	ActiveActionDelegate = Event;
	OnRequestLoadProfile.Broadcast(InSlotName);
}

void UProfileManagerSubsystem::RequestOpenLevel(const FName& InLevelName, FOnProfileActionFinished Event)
{
	if (bLoadingLevelInProgress)
		return;

	OpenLevelSpawnSettings = FOpenLevelPlayerSpawnSetting();
	LoadingLevelDelegate = Event;
	UGameplayStatics::OpenLevel(this, InLevelName);
}

void UProfileManagerSubsystem::RequestOpenLevelWithSpawnSetting(const FName& InLevelName, const FOpenLevelPlayerSpawnSetting& InSpawnSetting, FOnProfileActionFinished Event)
{
	if (bLoadingLevelInProgress)
		return;

	OpenLevelSpawnSettings = InSpawnSetting;
	LoadingLevelDelegate = Event;
	UGameplayStatics::OpenLevel(this, InLevelName);
}

void UProfileManagerSubsystem::SaveProfileData()
{
	UGameplayStatics::SaveGameToSlot(ProfileSaveObject, ProfileSaveSlot, 0);
	bProfileActionInProgress = false;
}

void UProfileManagerSubsystem::LoadProfileData()
{
	USaveGame* SaveObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(ProfileSaveSlot, 0))
	{
		SaveObject = UGameplayStatics::LoadGameFromSlot(ProfileSaveSlot, 0);
	}
	else
	{
		SaveObject = UGameplayStatics::CreateSaveGameObject(UProfilesSaveGame::StaticClass());
	}

	ProfileSaveObject = Cast<UProfilesSaveGame>(SaveObject);
	ProfilesName = ProfileSaveObject->Profiles;
	ActiveProfile = ProfileSaveObject->ActiveProfile;
	ProfilesSaveSlotsContainer = ProfileSaveObject->ProfilesSaveSlotsContainer;
}

void UProfileManagerSubsystem::InformSystemSaveDone(UObject* InListenerSystem)
{
	SavingObjects.Remove(InListenerSystem);

	// If all bound objects are done saving data, start saving profile data.
	if (SavingObjects.Num() == 0)
	{
		FSaveSlotInfo* SavingSlotInfo = FindOrAddSaveSlotForActiveProfile(CurrentSavingSlotName);
		SavingSlotInfo->SaveDateEpoch = FDateTime::Now().ToUnixTimestamp();
		ProfileSaveObject->ProfilesSaveSlotsContainer = ProfilesSaveSlotsContainer;
		SaveProfileData();

		if (ActiveActionDelegate.IsBound())
			ActiveActionDelegate.Execute();
	}
}

void UProfileManagerSubsystem::InformSystemLoadDone(UObject* InListenerSystem)
{
	LoadingObjects.Remove(InListenerSystem);

	if (LoadingObjects.Num() == 0)
	{
		bProfileActionInProgress = false;

		if (ActiveActionDelegate.IsBound())
			ActiveActionDelegate.Execute();
	}
}


// ~ start static function section
TSet<FString> UProfileManagerSubsystem::GetAllProfilesName(const UObject* WorldContextObject)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->GetAllProfilesName();
}

FString UProfileManagerSubsystem::GetActiveProfileName(const UObject* WorldContextObject)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->GetActiveProfileName();
}

bool UProfileManagerSubsystem::CreateProfile(const UObject* WorldContextObject, const FString& InProfileName)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->CreateProfile(InProfileName);
}

bool UProfileManagerSubsystem::RemoveProfile(const UObject* WorldContextObject, const FString& InProfileName)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->RemoveProfile(InProfileName);
}

bool UProfileManagerSubsystem::ActivateProfile(const UObject* WorldContextObject, const FString& InProfileName)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->ActivateProfile(InProfileName);
}

void UProfileManagerSubsystem::FindOrAddSaveSlotForActiveProfile(const UObject* WorldContextObject, const FString& InSlotName, FSaveSlotInfo& OutSaveSlot)
{
	OutSaveSlot = *UProfileManagerSubsystem::Get(WorldContextObject)->FindOrAddSaveSlotForActiveProfile(InSlotName);
}

bool UProfileManagerSubsystem::FindSaveSlotFromActiveProfile(const UObject* WorldContextObject, const FString& InSlotName, FSaveSlotInfo& OutSlotSaveData)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->FindSaveSlotForActiveProfile(InSlotName, OutSlotSaveData);
}

bool UProfileManagerSubsystem::RemoveSaveSlotFromActiveProfile(const UObject* WorldContextObject, const FString& InSlotName)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->RemoveSaveSlotFromActiveProfile(InSlotName);
}

TMap<FString, FSaveSlotInfo> UProfileManagerSubsystem::GetAllSaveDataForActiveProfile(const UObject* WorldContextObject)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->GetAllSaveDataForActiveProfile();
}

bool UProfileManagerSubsystem::DoesSaveSlotExist(const UObject* WorldContextObject, const FString& InSlotName)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->DoesSaveSlotExist(InSlotName);
}

void UProfileManagerSubsystem::RequestSaveGameDataInSlot(const UObject* WorldContextObject, const FString& InSlotName, ESlotSaveType InSaveType, FOnProfileActionFinished Event)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->RequestSaveGameDataInSlot(InSlotName, InSaveType, Event);
}

void UProfileManagerSubsystem::RequestLoadGameDataFromSlot(const UObject* WorldContextObject, const FString& InSlotName, FOnProfileActionFinished Event)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->RequestLoadGameDataFromSlot(InSlotName, Event);
}

void UProfileManagerSubsystem::SaveProfileData(const UObject* WorldContextObject)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->SaveProfileData();
}

void UProfileManagerSubsystem::LoadProfileData(const UObject* WorldContextObject)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->LoadProfileData();
}

void UProfileManagerSubsystem::InformSystemSaveDone(const UObject* WorldContextObject, UObject* InListenerSystem)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->InformSystemSaveDone(InListenerSystem);
}

void UProfileManagerSubsystem::InformSystemLoadDone(const UObject* WorldContextObject, UObject* InListenerSystem)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->InformSystemLoadDone(InListenerSystem);
}

bool UProfileManagerSubsystem::IsProfileActionInProgress(const UObject* WorldContextObject)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->IsProfileActionInProgress();
}

void UProfileManagerSubsystem::RequestOpenLevel(const UObject* WorldContextObject, const FName& InLevelName, FOnProfileActionFinished Event)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->RequestOpenLevel(InLevelName, Event);
}

void UProfileManagerSubsystem::RequestOpenLevelWithSpawnSetting(const UObject* WorldContextObject, const FName& InLevelName, FOpenLevelPlayerSpawnSetting InSpawnSetting, FOnProfileActionFinished Event)
{
	UProfileManagerSubsystem::Get(WorldContextObject)->RequestOpenLevelWithSpawnSetting(InLevelName, InSpawnSetting, Event);
}

FOpenLevelPlayerSpawnSetting UProfileManagerSubsystem::GetPlayerSpawnSetting(const UObject* WorldContextObject)
{
	return UProfileManagerSubsystem::Get(WorldContextObject)->OpenLevelSpawnSettings;
}

void UProfileManagerSubsystem::StartedLoadingLevel(const FWorldContext& WorldContext, const FString& InLevelName)
{
	bLoadingLevelInProgress = true;
	OnStartLoadingLevel.Broadcast(*InLevelName);
}

void UProfileManagerSubsystem::FinishedLoadingLevel(UWorld* World)
{
	const FString& CurrentLevelName = World->GetName();
	if (CurrentLevelName != "MainMenu")
	{
		ActiveLevelName = World->GetName();
	}

	bLoadingLevelInProgress = false;
	OnLoadingLevelFinished.Broadcast(*World->GetName());

	if (LoadingLevelDelegate.IsBound())
	{
		LoadingLevelDelegate.Execute();
		LoadingLevelDelegate.Unbind();
	}
}
// ~ End static function section