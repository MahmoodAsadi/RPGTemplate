// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ProfilesSaveGame.generated.h"


UENUM(BlueprintType)
enum class ESlotSaveType : uint8
{
	Manual,
	Auto,
	Quick
};

USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

public:

	FSaveSlotInfo() {}
	FSaveSlotInfo(const FString& InSlotName)
		: SaveSlotName(InSlotName)
	{}

	// Slot display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	FString SaveSlotName;

	// Save type, like Auto-Save, Quick-Save, Manual-Save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	ESlotSaveType SaveType = ESlotSaveType::Manual;

	// Time of the save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	int64 SaveDateEpoch = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	FString ActiveLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	FTransform PlayerLastTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	FString QuestSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlotInfo")
	FString InventorySlotName;

};

USTRUCT(BlueprintType)
struct FProfileSaveSlots
{
	GENERATED_BODY()

public:

	// Map of slot name and slot info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProfileSaveSlots")
	TMap<FString, FSaveSlotInfo> Slots;
};

/**
 * 
 */
UCLASS()
class PROFILEMANAGER_API UProfilesSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile Manager")
	TSet<FString> Profiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile Manager")
	FString ActiveProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile Manager")
	TMap<FString, FProfileSaveSlots> ProfilesSaveSlotsContainer;

};