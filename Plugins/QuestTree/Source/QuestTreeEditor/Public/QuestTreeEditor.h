// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

class UQuestTreeGraph;
class UQuestTreeEdGraph;
class UQuestTreeEdGraphNode;
class SGraphEditor;

/**
 * 
 */
class QUESTTREEEDITOR_API FQuestTreeEditor : public FAssetEditorToolkit, public FNotifyHook, public FGCObject
{
public:

	void Initialize(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UQuestTreeGraph* InQuestGraph);
	

	/** FAssetEditorToolkit Impl. */
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual bool OnRequestClose(EAssetEditorCloseReason InCloseReason) override;
	/** End FAssetEditorToolkit */

	// ~Begin IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	// ~End IToolkit interface

	/** FGCObject Impl. */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	/** End FGCObject */

	TArray<UQuestTreeEdGraphNode*> GetSelectedNodes() const;

private:

	void CreateEdQuestGraph();
	void CreateWidgets();
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();
	TSharedRef<IDetailsView> CreateDetailsPanel();

	/** Create logs tab widget */
	TSharedRef<SWidget> CreateLogsWidget();

	void CreateToolbar();
	void FillToolbarMenu(FToolBarBuilder& ToolbarBuilder);
	FSlateIcon GetCompileStatusImage() const;
	TSharedRef<FTabManager::FLayout> CreateLayout() const;

	/**
	* Registers the command list for the quest editor.
	*/
	void RegisterCommands();

	/** Called when the selection changes in the GraphEditor */
	void OnSelectedNodesChanged(const TSet<UObject*>& NewSelection);

	/** Select every node in the graph */
	void SelectAllNodes();
	/** Whether we can select every node */
	bool CanSelectAllNodes() const;

	/** Delete all selected nodes in the graph */
	void DeleteSelectedNodes();
	/** Whether we can delete all selected nodes */
	bool CanDeleteSelectedNodes() const;

	/** Copy all selected nodes in the graph */
	void CopySelectedNodes();
	/** Whether we can copy all selected nodes */
	bool CanCopySelectedNodes() const;

	/** Cut all selected nodes in the graph */
	void CutSelectedNodes();
	/** Whether we can cut all selected nodes */
	bool CanCutSelectedNodes() const;

	/** Paste nodes in the graph */
	void PasteNodes();
	/** Whether we can paste nodes */
	bool CanPasteNodes() const;

	/** Duplicate the currently selected nodes */
	void DuplicateNodes();
	/** Whether we are able to duplicate the currently selected nodes */
	bool CanDuplicateNodes() const;

	/** Called when the title of a node is changed */
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	/**
	 * Called when a node is double clicked
	 */
	void OnNodeDoubleClicked(UEdGraphNode* Node);

	/**
	 * Called when compile button pressed
	 */
	void OnCompile();

	void CreateLogFromString(FString InLog, EMessageSeverity::Type InSeverity);
	void CreateLogFromText(FText InLog, EMessageSeverity::Type InSeverity);
	void CreateLogsFromStrings(TMap<FString, EMessageSeverity::Type> InLogs);
	void CreateLogsFromCompileErrorInfos(const TArray<FQuestTreeCompileErrorInfo>& CompileInfo);
	EMessageSeverity::Type ConvertCompileStatusToLogSeverity(const EQuestTreeCompileStatus& InCompileStatus);
	void OnMessageLogLinkActivated(const class TSharedRef<IMessageToken>& Token);

	// Spawn editor tabs.
	TSharedRef<SDockTab> SpawnTab_GraphEditor(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Palette(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Log(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Find(const FSpawnTabArgs& Args);


	/** The Quest Graph being edited */
	UQuestTreeGraph* QuestTreeGraph;

	UQuestTreeEdGraph* QuestTreeEdGraph;

	/** The main graph viewport */
	TSharedPtr<SGraphEditor> GraphEditorWidget;

	/** The graph log widget */
	TSharedPtr<SWidget> LogWidget;

	// Output logs, with the log listing that it reflects
	TSharedPtr<IMessageLogListing> LogsListing;

	/** The list of UI commands for the editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;

	/** Details panel for the currently selected node */
	TSharedPtr<IDetailsView> PropertyDetailsPanel;
};