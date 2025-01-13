// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestTreeEditorModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#include "Graph/QuestTreeGraphNodeFactory.h"
#include "QuestTreeAssetTypeAction.h"
#include "QuestTreeStyle.h"

#define LOCTEXT_NAMESPACE "FQuestTreeEditorModule"

EAssetTypeCategories::Type FQuestTreeEditorModule::QuestTreeAssetCategory;

void FQuestTreeEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	RegisterAssetTypeActions();

	FEdGraphUtilities::RegisterVisualNodeFactory(MakeShared<FQuestTreeGraphNodeFactory>());
	FQuestTreeStyle::Initialize();
}

void FQuestTreeEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UnregisterAssetTypeActions();
	FQuestTreeStyle::Shutdown();
}

void FQuestTreeEditorModule::RegisterAssetTypeActions()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	QuestTreeAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("QuestTree")), LOCTEXT("QuestTreeCategory", "Quest Tree"));

	// Create Asset type actions.
	RegisteredAssetTypeActions.Emplace(MakeShareable(new FQuestTreeAssetTypeAction()));

	// Register Asset type actions.
	for (auto Action : RegisteredAssetTypeActions)
	{
		AssetTools.RegisterAssetTypeActions(Action);
	}
}

void FQuestTreeEditorModule::UnregisterAssetTypeActions()
{
	if (FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();

		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuestTreeEditorModule, QuestTreeEditor)