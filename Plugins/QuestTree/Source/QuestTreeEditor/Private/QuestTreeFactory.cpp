// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestTreeFactory.h"
#include "QuestTreeEditorModule.h"
#include "Graph/QuestTreeGraph.h"


UQuestTreeFactory::UQuestTreeFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UQuestTreeGraph::StaticClass();
}

UObject* UQuestTreeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UQuestTreeGraph* NewObjectAsset = NewObject<UQuestTreeGraph>(InParent, Class, Name, Flags | RF_Transactional);
	return NewObjectAsset;
}

uint32 UQuestTreeFactory::GetMenuCategories() const
{
	return FQuestTreeEditorModule::GetAssetCategory();
}

FText UQuestTreeFactory::GetDisplayName() const
{
	return FText::FromString("QuestTreeGraph");
}

FString UQuestTreeFactory::GetDefaultNewAssetName() const
{
	return FString("NewQuestTreeGraph");
}

bool UQuestTreeFactory::ShouldShowInNewMenu() const
{
	return true;
}