// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProfilesSaveGame.h"
#include "ProfileManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EOpenLevelPlayerTransformType : uint8
{
	// Uses default player start with None tag.
	Default,

	// Uses transform from where player was during player save.
	FromLoad,

	// Uses custom transform as player start.
	CustomTransform,

	// Uses custom player start tag.
	CustomPlayerStart
};
	

USTRUCT(BlueprintType)
struct FOpenLevelPlayerSpawnSetting
{
	GENERATED_BODY()

public:

	// Default constructor
	FOpenLevelPlayerSpawnSetting() {}

	
	FOpenLevelPlayerSpawnSetting(EOpenLevelPlayerTransformType InTransformType, FTransform InTransform)
		: TransformType(InTransformType),
		SpawnTransform(InTransform)
	{
	}

	FOpenLevelPlayerSpawnSetting(FString InPlayerStartName)
		: TransformType(EOpenLevelPlayerTransformType::CustomPlayerStart),
		PlayerStartName(InPlayerStartName)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnSetting")
	EOpenLevelPlayerTransformType TransformType = EOpenLevelPlayerTransformType::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnSetting", meta = (EditCondition = "TransformType == EOpenLevelPlayerTransformType::FromLoad || TransformType == EOpenLevelPlayerTransformType::CustomTransform", EditConditionHides))
	FTransform SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnSetting", meta = (EditCondition = "TransformType == EOpenLevelPlayerTransformType::CustomPlayerStart", EditConditionHides))
	FString PlayerStartName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestProfileAction, const FString&, InSlotName);
DECLARE_DYNAMIC_DELEGATE(FOnProfileActionFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReuqestLevelAction, const FString&, InLevelName);

/**
 * 
 */
UCLASS()
class PROFILEMANAGER_API UProfileManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	static UProfileManagerSubsystem* Get(const UObject* WorldContextObject);

	static FString ProfileSaveSlot;

	// ~ Start Subsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~ Finish Subsystem interface

	UPROPERTY(BlueprintAssignable)
	FOnRequestProfileAction OnRequestSaveProfile;

	UPROPERTY(BlueprintAssignable)
	FOnRequestProfileAction OnRequestLoadProfile;

	UPROPERTY(BlueprintAssignable)
	FOnReuqestLevelAction OnStartLoadingLevel;

	UPROPERTY(BlueprintAssignable)
	FOnReuqestLevelAction OnLoadingLevelFinished;

	TSet<FString> GetAllProfilesName() const { return ProfilesName; }
	FString GetActiveProfileName() const { return ActiveProfile; }
	bool CreateProfile(const FString& InProfileName);
	bool RemoveProfile(const FString& InProfileName);
	bool ActivateProfile(const FString& InProfileName);
	FSaveSlotInfo* FindOrAddSaveSlotForActiveProfile(const FString& InSlotName);
	bool FindSaveSlotForActiveProfile(const FString& InSlotName, FSaveSlotInfo& OutSlotSaveData);
	bool RemoveSaveSlotFromActiveProfile(const FString& InSlotName);
	TMap<FString, FSaveSlotInfo> GetAllSaveDataForActiveProfile() const;
	bool DoesSaveSlotExist(const FString& InSlotName) const;
	void RequestSaveGameDataInSlot(const FString& InSlotName, const ESlotSaveType& InSaveType, FOnProfileActionFinished Event);
	void RequestLoadGameDataFromSlot(const FString& InSlotName, FOnProfileActionFinished Event);
	void RequestOpenLevel(const FName& InLevelName, FOnProfileActionFinished Event);
	void RequestOpenLevelWithSpawnSetting(const FName& InLevelName, const FOpenLevelPlayerSpawnSetting& InSpawnSetting, FOnProfileActionFinished Event);
	void SaveProfileData();
	void LoadProfileData();
	void InformSystemSaveDone(UObject* InListenerSystem);
	void InformSystemLoadDone(UObject* InListenerSystem);
	bool IsProfileActionInProgress() const { return bProfileActionInProgress; }


	// Returns all profile names for this platform
	UFUNCTION(BlueprintPure, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static TSet<FString> GetAllProfilesName(const UObject* WorldContextObject);

	// Returns active profile name for this platform
	UFUNCTION(BlueprintPure, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static FString GetActiveProfileName(const UObject* WorldContextObject);

	// Creates a new profile
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InProfileName"))
	static bool CreateProfile(const UObject* WorldContextObject, const FString& InProfileName);

	// Remove existing profile
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InProfileName"))
	static bool RemoveProfile(const UObject* WorldContextObject, const FString& InProfileName);

	// Set a profile active
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InProfileName"))
	static bool ActivateProfile(const UObject* WorldContextObject, const FString& InProfileName);

	// Find or create a new save slot for active profile
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InSlotName"))
	static void FindOrAddSaveSlotForActiveProfile(const UObject* WorldContextObject, const FString& InSlotName, FSaveSlotInfo& OutSaveSlot);

	// Find save slot from active profile
	UFUNCTION(BlueprintPure, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InSlotName"))
	static bool FindSaveSlotFromActiveProfile(const UObject* WorldContextObject, const FString& InSlotName, FSaveSlotInfo& OutSlotSaveData);

	// Remove save slot from active profile
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InSlotName"))
	static bool RemoveSaveSlotFromActiveProfile(const UObject* WorldContextObject, const FString& InSlotName);

	// Get all save slot info for active profile
	UFUNCTION(BlueprintPure, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static TMap<FString, FSaveSlotInfo> GetAllSaveDataForActiveProfile(const UObject* WorldContextObject);

	// Checks if save slot name exist in active profile
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InSlotName"))
	static bool DoesSaveSlotExist(const UObject* WorldContextObject, const FString& InSlotName);

	// Request save game data in slot, inform all bound objects(systems) to save necessary data then save the profile data
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InSlotName"))
	static void RequestSaveGameDataInSlot(const UObject* WorldContextObject, const FString& InSlotName, ESlotSaveType InSaveType, FOnProfileActionFinished Event);

	// Request load game data from slot, inform all bound objects(systems) to load necessary data
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InSlotName"))
	static void RequestLoadGameDataFromSlot(const UObject* WorldContextObject, const FString& InSlotName, FOnProfileActionFinished Event);

	// Save profiles data
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static void SaveProfileData(const UObject* WorldContextObject);

	// Load profiles data
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static void LoadProfileData(const UObject* WorldContextObject);

	// Should be call from any objects(systems) which are bound to "OnRequestSaveProfile" to inform the system they have done saving data
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static void InformSystemSaveDone(const UObject* WorldContextObject, UObject* InListenerSystem);

	// Should be call from any objects(systems) which are bound to "OnRequestLoadProfile" to inform the system they have done loading data
	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static void InformSystemLoadDone(const UObject* WorldContextObject, UObject* InListenerSystem);

	// Checks if a save or load game data is in progress
	UFUNCTION(BlueprintPure, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static bool IsProfileActionInProgress(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InLevelName"))
	static void RequestOpenLevel(const UObject* WorldContextObject, const FName& InLevelName, FOnProfileActionFinished Event);

	UFUNCTION(BlueprintCallable, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InLevelName"))
	static void RequestOpenLevelWithSpawnSetting(const UObject* WorldContextObject, const FName& InLevelName, FOpenLevelPlayerSpawnSetting InSpawnSetting, FOnProfileActionFinished Event);

	UFUNCTION(BlueprintPure, Category = "Profile Manager", meta = (WorldContext = "WorldContextObject"))
	static FOpenLevelPlayerSpawnSetting GetPlayerSpawnSetting(const UObject* WorldContextObject);

protected:

	void StartedLoadingLevel(const FWorldContext& WorldContext, const FString& InLevelName);
	void FinishedLoadingLevel(UWorld* World);

	UPROPERTY(Transient)
	bool bProfileActionInProgress = false;

	UPROPERTY(Transient)
	bool bLoadingLevelInProgress = false;

	UPROPERTY(Transient)
	FString CurrentSavingSlotName;

	UPROPERTY(Transient)
	TArray<UObject*> SavingObjects;

	UPROPERTY(Transient)
	TArray<UObject*> LoadingObjects;

	UPROPERTY()
	UProfilesSaveGame* ProfileSaveObject = nullptr;

	UPROPERTY()
	TSet<FString> ProfilesName;

	UPROPERTY()
	FString ActiveProfile;

	UPROPERTY()
	FString ActiveLevelName;

	UPROPERTY()
	FOpenLevelPlayerSpawnSetting OpenLevelSpawnSettings;

	UPROPERTY()
	TMap<FString, FProfileSaveSlots> ProfilesSaveSlotsContainer;

	UPROPERTY()
	FOnProfileActionFinished ActiveActionDelegate;

	UPROPERTY()
	FOnProfileActionFinished LoadingLevelDelegate;

};