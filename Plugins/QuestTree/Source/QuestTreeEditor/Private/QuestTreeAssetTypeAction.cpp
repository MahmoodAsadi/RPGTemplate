// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestTreeAssetTypeAction.h"
#include "QuestTreeEditor.h"
#include "QuestTreeEditorModule.h"
#include "Graph/QuestTreeGraph.h"

#define LOCTEXT_NAMESPACE "QuestTreeAssetTypeAction"


FText FQuestTreeAssetTypeAction::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "QuestTreeGraphAssetTypeActions", "QuestTree Graph");
}

FColor FQuestTreeAssetTypeAction::GetTypeColor() const
{
	return FColor::Purple;
}

uint32 FQuestTreeAssetTypeAction::GetCategories()
{
	return FQuestTreeEditorModule::GetAssetCategory();
}

UClass* FQuestTreeAssetTypeAction::GetSupportedClass() const
{
	return UQuestTreeGraph::StaticClass();
}

void FQuestTreeAssetTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	//Get the toolkit mode
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	//Attempt to open an asset editor for the object
	for (UObject* Object : InObjects)
	{
		if (UQuestTreeGraph* TargetQuestGraph = Cast<UQuestTreeGraph>(Object))
		{
			TSharedRef<FQuestTreeEditor> QuestTreeEditor = MakeShared<FQuestTreeEditor>();
			QuestTreeEditor->Initialize(Mode, EditWithinLevelEditor, TargetQuestGraph);
		}
	}
}

#undef LOCTEXT_NAMESPACE