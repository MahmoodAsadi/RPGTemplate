// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "QuestTreeFactory.generated.h"

/**
 * 
 */
UCLASS()
class QUESTTREEEDITOR_API UQuestTreeFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
	virtual bool ShouldShowInNewMenu() const override;
	// End of UFactory interface

};
