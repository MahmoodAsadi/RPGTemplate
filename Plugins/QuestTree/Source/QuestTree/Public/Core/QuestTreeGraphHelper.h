// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestTreeGraphHelper.generated.h"

USTRUCT()
struct FQuestTreePinMaker
{
	GENERATED_BODY()
public:

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FString ToolTip;

	UPROPERTY()
	FLinearColor DefaultColor = FLinearColor::Gray;

	FQuestTreePinMaker() = default;

	// Make normal Pin
	FQuestTreePinMaker(const FName& InName, const FString& InToolTip)
		: Name(InName)
		, ToolTip(InToolTip)
		, bIsValid(true)
	{
	}

	// Make normal Pin with default value
	FQuestTreePinMaker(const FName& InName, const FString& InToolTip, const FLinearColor& InDefaultColor)
		: Name(InName)
		, ToolTip(InToolTip)
		, DefaultColor(InDefaultColor)
		, bIsValid(true)
	{
	}

private:

	UPROPERTY()
	bool bIsValid = false;
};


namespace FQuestTreeEditorCommon
{
	const FString ContextIdentifier = TEXT("QuestTreeEditorContext");
	const FString GraphSchemaActions = TEXT("QuestTreeGraphSchemaActions");
	const FName GraphPinCategory = TEXT("QuestTreePinType");
};