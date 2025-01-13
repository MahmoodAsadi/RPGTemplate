// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestTreeEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraphSchema.h"
#include "GraphEditor.h"
#include "Framework/Commands/UICommandList.h"
#include "PropertyEditorModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "EdGraphUtilities.h"
#include "HAL/PlatformApplicationMisc.h"
#include "SNodePanel.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "MessageLogInitializationOptions.h"
#include "Misc/UObjectToken.h"

#include "Core/QuestTreeGraphHelper.h"
#include "Graph/QuestTreeEdGraph.h"
#include "Graph/QuestTreeEdGraphNode.h"
#include "Graph/QuestTreeEdGraphSchema.h"
#include "Graph/QuestTreeGraph.h"
#include "Graph/QuestTreeNode.h"
#include "Graph/QuestTreeNodePin.h"

#define LOCTEXT_NAMESPACE "QuestTreeEditor"

const FName QuestTreeEditorAppName = FName(TEXT("QuestTreeEditorApp"));

namespace FQuestTreeEditorTapIDs
{
	const FName GraphEditorID = FName(TEXT("GraphEditor"));
	const FName PaletteID = FName(TEXT("Palette"));
	const FName DetailsID = FName(TEXT("Details"));
	const FName LogID = FName(TEXT("Log"));
	const FName FindID = FName(TEXT("Find"));
}


void FQuestTreeEditor::Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuestTreeGraph* InQuestGraph)
{
	QuestTreeGraph = InQuestGraph;
	CreateEdQuestGraph();
	
	FGenericCommands::Register();
	//FGraphEditorCommands::Register();

	CreateWidgets();

	//Define Layout 
	const TSharedRef<FTabManager::FLayout> DefaultLayout = CreateLayout();

	// Initialize the asset editor
	InitAssetEditor(Mode, InitToolkitHost, QuestTreeEditorAppName, DefaultLayout, true, true, InQuestGraph);
}

FLinearColor FQuestTreeEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FName FQuestTreeEditor::GetToolkitFName() const
{
	return FName(TEXT("QuestTreeEditor"));
}

FText FQuestTreeEditor::GetBaseToolkitName() const
{
	return LOCTEXT("QuestTreeEditorBaseToolkitName", "Quest Tree Editor");
}

FString FQuestTreeEditor::GetWorldCentricTabPrefix() const
{
	return FString("QuestTreeEditor");
}

FText FQuestTreeEditor::GetToolkitName() const
{
	if (QuestTreeGraph)
	{
		return FText::FromString(QuestTreeGraph->GetName());
	}

	return LOCTEXT("QuestTreeEditorToolkitName", "Quest Tree Editor");
}

FText FQuestTreeEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(QuestTreeGraph);
}

bool FQuestTreeEditor::OnRequestClose(EAssetEditorCloseReason InCloseReason)
{
	return FAssetEditorToolkit::OnRequestClose(InCloseReason);
}

void FQuestTreeEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_QuestTreeEditor", "QuestTreeEditor"));
	const TSharedRef<FWorkspaceItem>& WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FQuestTreeEditorTapIDs::GraphEditorID, FOnSpawnTab::CreateSP(this, &FQuestTreeEditor::SpawnTab_GraphEditor))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "FullBlueprintEditor.SwitchToScriptingMode"));

	InTabManager->RegisterTabSpawner(FQuestTreeEditorTapIDs::PaletteID, FOnSpawnTab::CreateSP(this, &FQuestTreeEditor::SpawnTab_Palette))
		.SetDisplayName(LOCTEXT("PaletteTab", "Palette"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.Palette"));

	InTabManager->RegisterTabSpawner(FQuestTreeEditorTapIDs::DetailsID, FOnSpawnTab::CreateSP(this, &FQuestTreeEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FQuestTreeEditorTapIDs::LogID, FOnSpawnTab::CreateSP(this, &FQuestTreeEditor::SpawnTab_Log))
		.SetDisplayName(LOCTEXT("LogTab", "Log"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.CompilerResults"));

	InTabManager->RegisterTabSpawner(FQuestTreeEditorTapIDs::FindID, FOnSpawnTab::CreateSP(this, &FQuestTreeEditor::SpawnTab_Find))
		.SetDisplayName(LOCTEXT("FindTab", "Find"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.FindResults"));
}

void FQuestTreeEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(FQuestTreeEditorTapIDs::GraphEditorID);
	InTabManager->UnregisterTabSpawner(FQuestTreeEditorTapIDs::PaletteID);
	InTabManager->UnregisterTabSpawner(FQuestTreeEditorTapIDs::DetailsID);
	InTabManager->UnregisterTabSpawner(FQuestTreeEditorTapIDs::LogID);
	InTabManager->UnregisterTabSpawner(FQuestTreeEditorTapIDs::FindID);
}

void FQuestTreeEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	check(QuestTreeGraph);
	UEdGraph* TargetEdGraph = QuestTreeGraph->GetEdGraph();
	check(TargetEdGraph);

	Collector.AddReferencedObject(QuestTreeGraph);
	Collector.AddReferencedObject(TargetEdGraph);
}

FString FQuestTreeEditor::GetReferencerName() const
{
	return TEXT("FQuestTreeEditor");
}

TArray<UQuestTreeEdGraphNode*> FQuestTreeEditor::GetSelectedNodes() const
{
	return TArray<UQuestTreeEdGraphNode*>();
	TArray<UQuestTreeEdGraphNode*> SelectedNodes;
	const FGraphPanelSelectionSet SelectedObjects = GraphEditorWidget->GetSelectedNodes();

	for (UObject* Object : SelectedObjects)
	{
		if (UQuestTreeEdGraphNode* Node = Cast<UQuestTreeEdGraphNode>(Object))
		{
			SelectedNodes.AddUnique(Node);
		}
	}

	SelectedNodes.Remove(nullptr);
	return SelectedNodes;
}

void FQuestTreeEditor::CreateEdQuestGraph()
{
	if (!QuestTreeGraph->GetEdGraph())
	{
		//Create the Ed graph
		UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(
			QuestTreeGraph,
			NAME_None,
			UQuestTreeEdGraph::StaticClass(),
			UQuestTreeEdGraphSchema::StaticClass()
		);
		check(NewGraph);
		NewGraph->bAllowDeletion = false;
		QuestTreeGraph->SetEdGraph(NewGraph);
		QuestTreeEdGraph = Cast<UQuestTreeEdGraph>(NewGraph);
		QuestTreeEdGraph->SetQuestTreeGraph(QuestTreeGraph);

		//Spawn initial nodes
		const UEdGraphSchema* GraphSchema = NewGraph->GetSchema();
		check(GraphSchema);
		GraphSchema->CreateDefaultNodesForGraph(*NewGraph);
	}

	if (!QuestTreeEdGraph)
	{
		QuestTreeEdGraph = Cast<UQuestTreeEdGraph>(QuestTreeGraph->GetEdGraph());
	}
}

void FQuestTreeEditor::CreateWidgets()
{
	GraphEditorWidget = CreateGraphEditorWidget();
	PropertyDetailsPanel = CreateDetailsPanel();
	LogWidget = CreateLogsWidget();
	LogsListing->OnMessageTokenClicked().AddSP(this, &FQuestTreeEditor::OnMessageLogLinkActivated);
	CreateLogFromString("Graph successfully compiled.", EMessageSeverity::Info);
	CreateToolbar();
}

TSharedRef<SGraphEditor> FQuestTreeEditor::CreateGraphEditorWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("QuestTreeEditorCornerText", "Quest Graph");

	//Register editor commands
	RegisterCommands();

	// Bind to editor delegates.
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FQuestTreeEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FQuestTreeEditor::OnNodeTitleCommitted);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FQuestTreeEditor::OnNodeDoubleClicked);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(QuestTreeGraph->GetEdGraph())
		.GraphEvents(InEvents)
		.ShowGraphStateOverlay(true);
}

TSharedRef<IDetailsView> FQuestTreeEditor::CreateDetailsPanel()
{
	//Get property module
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	return PropertyEditorModule.CreateDetailView(DetailsViewArgs);
}

TSharedRef<SWidget> FQuestTreeEditor::CreateLogsWidget()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions LogOptions;

	LogOptions.bShowPages = false;
	LogOptions.bShowFilters = false;
	LogOptions.bAllowClear = false;
	LogOptions.MaxPageCount = 1;
	LogsListing = MessageLogModule.CreateLogListing("QuestTreeEditorStats", LogOptions);
	return MessageLogModule.CreateLogListingWidget(LogsListing.ToSharedRef());
}

void FQuestTreeEditor::CreateToolbar()
{
	//Set up the toolbar exteder
	TSharedPtr<FExtender> Extender = MakeShareable(new FExtender);

	//Create callback delegate and register with extender
	FToolBarExtensionDelegate ExtensionDelegate = FToolBarExtensionDelegate::CreateSP(this, &FQuestTreeEditor::FillToolbarMenu);

	Extender->AddToolBarExtension(
		FName("Asset"),
		EExtensionHook::Before,
		GetToolkitCommands(),
		ExtensionDelegate
	);

	//Add the extender to the toolbar
	AddToolbarExtender(Extender);
}

void FQuestTreeEditor::FillToolbarMenu(FToolBarBuilder& ToolbarBuilder)
{
	//Build up toolbar 
	ToolbarBuilder.BeginSection("QuestGraph");
	{
		FText CompileLabel = LOCTEXT("CompileLabel", "Compile");
		FUIAction CompileAction = FUIAction(FExecuteAction::CreateSP(this, &FQuestTreeEditor::OnCompile));

		ToolbarBuilder.AddToolBarButton(
			CompileAction,
			NAME_None,
			CompileLabel,
			CompileLabel,
			TAttribute<FSlateIcon>(this, &FQuestTreeEditor::GetCompileStatusImage)
		);
	}
	ToolbarBuilder.EndSection();
}

FSlateIcon FQuestTreeEditor::GetCompileStatusImage() const
{
	check(QuestTreeGraph);

	//Set icon names
	static const FName CompileStatusBackground("Blueprint.CompileStatus.Background");
	static const FName CompileStatusCompiled("Blueprint.CompileStatus.Overlay.Good");
	static const FName CompileStatusUncompiled("Blueprint.CompileStatus.Overlay.Unknown");
	static const FName CompileStatusWarning("Blueprint.CompileStatus.Overlay.Warning");
	static const FName CompileStatusFailed("Blueprint.CompileStatus.Overlay.Error");

	//Combine icons and return as appropriate 
	switch (QuestTreeGraph->GetCompileStatus())
	{
	case EQuestTreeCompileStatus::Compiled:
		return FSlateIcon(
			FAppStyle::GetAppStyleSetName(),
			CompileStatusBackground,
			NAME_None,
			CompileStatusCompiled
		);

	case EQuestTreeCompileStatus::Uncompiled:
		return FSlateIcon(
			FAppStyle::GetAppStyleSetName(),
			CompileStatusBackground,
			NAME_None,
			CompileStatusUncompiled
		);

	case EQuestTreeCompileStatus::Warning:
		return FSlateIcon(
			FAppStyle::GetAppStyleSetName(),
			CompileStatusBackground,
			NAME_None,
			CompileStatusWarning
		);

	case EQuestTreeCompileStatus::Failed:
		return FSlateIcon(
			FAppStyle::GetAppStyleSetName(),
			CompileStatusBackground,
			NAME_None,
			CompileStatusFailed
		);

	default: //Default to failed: should never happen
		return FSlateIcon(
			FAppStyle::GetAppStyleSetName(),
			CompileStatusBackground,
			NAME_None,
			CompileStatusFailed
		);
	}
}

TSharedRef<FTabManager::FLayout> FQuestTreeEditor::CreateLayout() const
{
	FName LayoutName = "QuestTreeEditorDefaultLayout";

	return FTabManager::NewLayout(LayoutName)
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.15f)
				->AddTab(FQuestTreeEditorTapIDs::PaletteID, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.65f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->AddTab(FQuestTreeEditorTapIDs::GraphEditorID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetHideTabWell(true)
					->AddTab(FQuestTreeEditorTapIDs::LogID, ETabState::OpenedTab)
					->AddTab(FQuestTreeEditorTapIDs::FindID, ETabState::OpenedTab)
				)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.2f)
				->AddTab(FQuestTreeEditorTapIDs::DetailsID, ETabState::OpenedTab)
			)
		);
}

void FQuestTreeEditor::RegisterCommands()
{
	//Don't double register commands
	if (GraphEditorCommands.IsValid())
		return;

	GraphEditorCommands = MakeShareable(new FUICommandList);

	// Node Behavior commands
	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateSP(this, &FQuestTreeEditor::SelectAllNodes),
		FCanExecuteAction::CreateSP(this, &FQuestTreeEditor::CanSelectAllNodes));

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &FQuestTreeEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FQuestTreeEditor::CanDeleteSelectedNodes));

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateSP(this, &FQuestTreeEditor::CopySelectedNodes),
		FCanExecuteAction::CreateSP(this, &FQuestTreeEditor::CanCopySelectedNodes));

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateSP(this, &FQuestTreeEditor::CutSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FQuestTreeEditor::CanCutSelectedNodes));

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateSP(this, &FQuestTreeEditor::PasteNodes),
		FCanExecuteAction::CreateSP(this, &FQuestTreeEditor::CanPasteNodes));

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &FQuestTreeEditor::DuplicateNodes),
		FCanExecuteAction::CreateSP(this, &FQuestTreeEditor::CanDuplicateNodes));
}

void FQuestTreeEditor::OnSelectedNodesChanged(const TSet<UObject*>& NewSelection)
{
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;

	for (UObject* Object : NewSelection)
	{
		if (UQuestTreeEdGraphNode* QuestGraphNode = Cast<UQuestTreeEdGraphNode>(Object))
		{
			SelectedObjects.Add(QuestGraphNode->GetQuestTreeNode());
			
			// Start debugging purposes
			/*UE_LOG(LogTemp, Error, TEXT("Selected Node = [%s]"), *QuestGraphNode->GetName());
			if (UQuestTreeNode* QuestTreeNode = QuestGraphNode->GetQuestTreeNode())
			{
				for (UQuestTreeNodePin* Pin : QuestTreeNode->GetOutputPins())
				{
					if (!Pin)
						continue;

					if (Pin->ConnectedPins.Num() > 0)
					{
						for (UQuestTreeNodePin* LinkedPin : Pin->ConnectedPins)
						{
							if (!LinkedPin)
								continue;

							UE_LOG(LogTemp, Warning, TEXT("[%s] is connected to [%s] which node is [%s]"), *Pin->EdGraphPinName.ToString(), *LinkedPin->EdGraphPinName.ToString(), *LinkedPin->OwningNode->GetName());
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Connected pin count for [%s] is zero"), *Pin->EdGraphPinName.ToString());
					}
				}
			}*/
			// End debugging purposes
		}
		else if (UEdGraphNode* GraphNode = Cast<UEdGraphNode>(Object))
		{
			SelectedObjects.Add(GraphNode);
		}
	}

	PropertyDetailsPanel->SetObjects(SelectedObjects, /*bForceRefresh=*/true);
	GetTabManager()->TryInvokeTab(FQuestTreeEditorTapIDs::DetailsID);
}

void FQuestTreeEditor::SelectAllNodes()
{
	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->SelectAllNodes();
	}
}

bool FQuestTreeEditor::CanSelectAllNodes() const
{
	return GraphEditorWidget.IsValid();
}

void FQuestTreeEditor::DeleteSelectedNodes()
{
	if (!GraphEditorWidget.IsValid() || !IsValid(QuestTreeEdGraph))
	{
		return;
	}

	const FScopedTransaction Transaction(*FQuestTreeEditorCommon::ContextIdentifier, LOCTEXT("QuestTreeEditorDeleteTransaction", "QuestTree Editor: DeleteNode"), nullptr);
	QuestTreeEdGraph->Modify();
	QuestTreeGraph->Modify();
	bool bDeletedAny = false;
	const FGraphPanelSelectionSet SelectedNodes = GraphEditorWidget->GetSelectedNodes();
	GraphEditorWidget->ClearSelectionSet();

	for (UObject* Object : SelectedNodes)
	{
		UEdGraphNode* EdNode = CastChecked<UEdGraphNode>(Object);
		if (!EdNode->CanUserDeleteNode())
		{
			continue;
		}

		if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(EdNode))
		{
			QuestTreeGraph->RemoveNode(QuestTreeEdNode->GetQuestTreeNode());
		}

		EdNode->DestroyNode();
		bDeletedAny = true;
	}

	if (bDeletedAny)
	{
		QuestTreeEdGraph->NotifyGraphChanged();
		QuestTreeEdGraph->MarkCompileStateDirty();
	}
}

bool FQuestTreeEditor::CanDeleteSelectedNodes() const
{
	if (GraphEditorWidget.IsValid())
	{
		for (UObject* Object : GraphEditorWidget->GetSelectedNodes())
		{
			UEdGraphNode* GraphNode = CastChecked<UEdGraphNode>(Object);
			if (GraphNode->CanUserDeleteNode())
			{
				return true;
			}
		}
	}
	
	return false;
}

void FQuestTreeEditor::CopySelectedNodes()
{
	if (!GraphEditorWidget.IsValid())
		return;

	FGraphPanelSelectionSet Selected = GraphEditorWidget->GetSelectedNodes();

	//Copy any nodes that allow 
	for (FGraphPanelSelectionSet::TIterator SelectedIter(Selected); SelectedIter; ++SelectedIter)
	{
		//Convert selected item to a node 
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);

		if (Node && Node->CanDuplicateNode())
		{ 
			/*
			* In PrepareForCopying, we temporary change the QuestTreeNode outer to the EdGraphNode (instead of original QuestTreeGraph)
			* to make sure we also copy the actual node (QuestTreeNode)
			* After copy the node to the clipboard, we change back the node owner to the QuestTreeGraph again, @See UQuestTreeEdGraphNode::PostCopyNode()
			* note that owner of QuestTreeEdGraphNode is QuestTreeEdGraph and owner of QuestTreeNode is QuestTreeGraph.
			*/
			Node->PrepareForCopying();
		}
		else
		{
			SelectedIter.RemoveCurrent();
		}
	}

	FString Exported;
	FEdGraphUtilities::ExportNodesToText(Selected, Exported);
	FPlatformApplicationMisc::ClipboardCopy(*Exported);

	// Make sure the QuestTreeGraph remains the owner of the copied nodes
	for (auto It = Selected.CreateIterator(); It; ++It)
	{
		if (auto* Node = Cast<UQuestTreeEdGraphNode>(*It))
		{
			Node->ResetGraphNodeOwner();
		}
	}
}

bool FQuestTreeEditor::CanCopySelectedNodes() const
{
	if (GraphEditorWidget.IsValid())
	{
		for (UObject* Object : GraphEditorWidget->GetSelectedNodes())
		{
			if (UEdGraphNode* GraphNode = CastChecked<UEdGraphNode>(Object))
			{
				if (GraphNode->CanDuplicateNode())
				{
					return true;
				}
			}
		}
	}

	return false;
}

void FQuestTreeEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedNodes();
}

bool FQuestTreeEditor::CanCutSelectedNodes() const
{
	return CanCopySelectedNodes() && CanDeleteSelectedNodes();
}

void FQuestTreeEditor::PasteNodes()
{
	if (GraphEditorWidget.IsValid() == false)
	{
		return;
	}

	//Set up transaction
	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	UEdGraph* EdGraph = GraphEditorWidget->GetCurrentGraph();
	EdGraph->Modify();

	//Clear selection to make room for pasted items to be selected
	GraphEditorWidget->ClearSelectionSet();

	const FVector2D PasteLocation = GraphEditorWidget->GetPasteLocation();

	//Retrieve import text from clipboard 
	FString ImportText;
	FPlatformApplicationMisc::ClipboardPaste(ImportText);

	//Retrieve nodes from text 
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, ImportText, PastedNodes);

	//Average node position for group movement 
	FVector2D AverageNodePosition(0.f, 0.f);
	for (UEdGraphNode* Node : PastedNodes)
	{
		AverageNodePosition.X += Node->NodePosX;
		AverageNodePosition.Y += Node->NodePosY;
	}
	AverageNodePosition /= float(PastedNodes.Num());

	//Place each node 
	for (UEdGraphNode* Node : PastedNodes)
	{
		Node->NodePosX = Node->NodePosX - AverageNodePosition.X + PasteLocation.X;
		Node->NodePosY = Node->NodePosY - AverageNodePosition.Y + PasteLocation.Y;
		Node->SnapToGrid(SNodePanel::GetSnapGridSize());
		Node->CreateNewGuid();

		if (UQuestTreeEdGraphNode* QuestGraphNode = Cast<UQuestTreeEdGraphNode>(Node))
		{
			QuestGraphNode->ResetGraphNodeOwner();
			QuestTreeGraph->PostCopyNode(QuestGraphNode->GetQuestTreeNode());
		}

		GraphEditorWidget->SetNodeSelection(Node, true);
	}

	//Update graph editor 
	GraphEditorWidget->NotifyGraphChanged();
	QuestTreeEdGraph->NotifyGraphChanged();
	QuestTreeEdGraph->MarkCompileStateDirty();
	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
	}
}

bool FQuestTreeEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(QuestTreeEdGraph, ClipboardContent);
}

void FQuestTreeEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FQuestTreeEditor::CanDuplicateNodes() const
{
	return CanCopySelectedNodes();
}

void FQuestTreeEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
}

void FQuestTreeEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
}

void FQuestTreeEditor::OnCompile()
{
	TArray<FQuestTreeCompileErrorInfo> FoundIssues;
	QuestTreeGraph->SetCompileStatus(QuestTreeEdGraph->CompileGraph(FoundIssues));
	
	// Calling NotifyGraphChanged to make sure node warnings and errors display correctly. 
	GraphEditorWidget->NotifyGraphChanged();

	CreateLogsFromCompileErrorInfos(FoundIssues);
}

void FQuestTreeEditor::CreateLogFromString(FString InLog, EMessageSeverity::Type InSeverity)
{
	TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(InSeverity);
	Message->AddToken(FTextToken::Create(FText::FromString(InLog)));
	LogsListing->ClearMessages();
	LogsListing->AddMessage(Message);
}

void FQuestTreeEditor::CreateLogFromText(FText InLog, EMessageSeverity::Type InSeverity)
{
	TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(InSeverity);
	Message->AddToken(FTextToken::Create(InLog));
	LogsListing->ClearMessages();
	LogsListing->AddMessage(Message);
}

void FQuestTreeEditor::CreateLogsFromStrings(TMap<FString, EMessageSeverity::Type> InLogs)
{
	TArray<TSharedRef<FTokenizedMessage>> Messages;

	for (const TPair<FString, EMessageSeverity::Type>& Message : InLogs)
	{
		TSharedRef<FTokenizedMessage> Line = FTokenizedMessage::Create(Message.Value);
		Line->AddToken(FTextToken::Create(FText::FromString(Message.Key)));
		Messages.Add(Line);
	}

	LogsListing->ClearMessages();
	LogsListing->AddMessages(Messages);
}

void FQuestTreeEditor::CreateLogsFromCompileErrorInfos(const TArray<FQuestTreeCompileErrorInfo>& CompileInfo)
{
	if (CompileInfo.Num() == 0)
	{
		CreateLogFromString("Graph successfully compiled.", EMessageSeverity::Info);
		return;
	}

	LogsListing->ClearMessages();
	TArray<TSharedRef<FTokenizedMessage>> Messages;

	for (const FQuestTreeCompileErrorInfo& Info : CompileInfo)
	{
		TSharedRef<FTokenizedMessage> Line = FTokenizedMessage::Create(ConvertCompileStatusToLogSeverity(Info.Status));
		Line->AddToken(FTextToken::Create(Info.IssueDescription));
		Line->SetMessageLink(FUObjectToken::Create(Info.OwningNode));
		Messages.Add(Line);
	}
	
	LogsListing->AddMessages(Messages);
	GetTabManager()->TryInvokeTab(FQuestTreeEditorTapIDs::LogID);
}

EMessageSeverity::Type FQuestTreeEditor::ConvertCompileStatusToLogSeverity(const EQuestTreeCompileStatus& InCompileStatus)
{
	switch (InCompileStatus)
	{
		case EQuestTreeCompileStatus::Uncompiled:
			return EMessageSeverity::Info;

		case EQuestTreeCompileStatus::Compiled:
			return EMessageSeverity::Info;

		case EQuestTreeCompileStatus::Warning:
			return EMessageSeverity::Warning;

		case EQuestTreeCompileStatus::Failed:
			return EMessageSeverity::Error;
	}
	return EMessageSeverity::Info;
}

void FQuestTreeEditor::OnMessageLogLinkActivated(const TSharedRef<IMessageToken>& Token)
{
	const TSharedRef<FUObjectToken> UObjectToken = StaticCastSharedRef<FUObjectToken>(Token);
	if (!UObjectToken->GetObject().IsValid())
		return;

	if (UEdGraphNode* RelevantNode = Cast<UEdGraphNode>(UObjectToken->GetObject().Get()))
		GraphEditorWidget->JumpToNode(RelevantNode);
}

TSharedRef<SDockTab> FQuestTreeEditor::SpawnTab_GraphEditor(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("QuestTreeGraphTitle", "Graph"))
		.OnCanCloseTab_Lambda([]() { return false; })
		.TabColorScale(GetTabColorScale())
		[
			GraphEditorWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FQuestTreeEditor::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("PaletteTab", "Palette"));
}

TSharedRef<SDockTab> FQuestTreeEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab", "Details"))
		.TabColorScale(GetTabColorScale())
		[
			PropertyDetailsPanel.ToSharedRef()
		];
}

TSharedRef<SDockTab> FQuestTreeEditor::SpawnTab_Log(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("LogsTab", "Logs"))
		.TabColorScale(GetTabColorScale())
		[
			LogWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FQuestTreeEditor::SpawnTab_Find(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("FindTab", "Find Results"))
		.TabColorScale(GetTabColorScale());
}

#undef LOCTEXT_NAMESPACE