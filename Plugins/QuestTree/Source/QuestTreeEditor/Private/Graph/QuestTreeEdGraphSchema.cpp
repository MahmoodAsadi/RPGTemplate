// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeEdGraphSchema.h"
#include "EdGraph/EdGraph.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "Toolkits/ToolkitManager.h"

#include "Graph/QuestTreeConnectionDrawingPolicy.h"
#include "Graph/QuestTreeEdGraph.h"
#include "Graph/QuestTreeEdGraphNode.h"
#include "Graph/QuestTreeGraph.h"
#include "Graph/QuestTreeGraphSchemaActions.h"
#include "Graph/QuestTreeNode.h"
#include "QuestTreeEditor.h"

#define LOCTEXT_NAMESPACE "QuestTreeGraphSchema"

TArray<UClass*> UQuestTreeEdGraphSchema::QuestTreeNodeClasses;
bool UQuestTreeEdGraphSchema::bQuestTreeNodeClassesInitialized = false;

FText GetClassCategory(UClass* InClass)
{
	return InClass->GetMetaDataText(TEXT("Category"), TEXT("UObjectCategory"), InClass->GetFullGroupName(false));
}

bool CategorizePinsByDirection(const UEdGraphPin* PinA, const UEdGraphPin* PinB, const UEdGraphPin*& InputPin, const UEdGraphPin*& OutputPin)
{
	InputPin = nullptr;
	OutputPin = nullptr;

	if ((PinA->Direction == EGPD_Input) && (PinB->Direction == EGPD_Output))
	{
		InputPin = PinA;
		OutputPin = PinB;
		return true;
	}
	else if ((PinB->Direction == EGPD_Input) && (PinA->Direction == EGPD_Output))
	{
		InputPin = PinB;
		OutputPin = PinA;
		return true;
	}
	else
	{
		return false;
	}
}

void UQuestTreeEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	GetQuestTreeNodeActions(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);
}

void UQuestTreeEdGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UQuestTreeEdGraph* QuestTreeEdGraph = Cast<UQuestTreeEdGraph>(&Graph);
	UQuestTreeGraph* QuestTree = QuestTreeEdGraph->GetQuestTreeGraph();
	FGraphNodeCreator<UQuestTreeEdGraphNode> GraphRootNodeCreator(Graph);
	UQuestTreeEdGraphNode* GraphRootNode = GraphRootNodeCreator.CreateNode(false);
	UQuestTreeNode_Root* RootNode = NewObject<UQuestTreeNode_Root>(QuestTree, UQuestTreeNode_Root::StaticClass(), "RootNode", RF_Transactional);
	GraphRootNode->Construct(RootNode);
	GraphRootNodeCreator.Finalize();
	SetNodeMetaData(GraphRootNode, FNodeMetadata::DefaultGraphNode);
	QuestTreeEdGraph->SetEdGraphRootNode(GraphRootNode);
	QuestTree->SetRootNode(RootNode);
	Graph.NotifyGraphChanged();
	QuestTreeEdGraph->MarkCompileStateDirty();
}

const FPinConnectionResponse UQuestTreeEdGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	check(PinA && PinB);
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinConnection", "Both pins are on same node"));
	}

	// Compare the directions
	const UEdGraphPin* InputPin = nullptr;
	const UEdGraphPin* OutputPin = nullptr;

	if (!CategorizePinsByDirection(PinA, PinB, InputPin, OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinConnection", "Directions are not compatible"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

bool UQuestTreeEdGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	check(PinA && PinB);
	bool bModified = UEdGraphSchema::TryCreateConnection(PinA, PinB);
	if (!bModified)
		return false;

	PinA->GetOwningNode()->GetGraph()->NotifyGraphChanged();
	Cast<UQuestTreeEdGraph>(PinA->GetOwningNode()->GetGraph())->MarkCompileStateDirty();

	return bModified;
}

void UQuestTreeEdGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	TSet<UEdGraphNode*> NodeList;
	NodeList.Add(&TargetNode);

	// Iterate over each pin and break all links
	for (TArray<UEdGraphPin*>::TIterator PinIt(TargetNode.Pins); PinIt; ++PinIt)
	{
		UEdGraphPin* TargetPin = *PinIt;
		if (TargetPin != nullptr && !TargetPin->bHidden)
		{
			// Keep track of which node(s) the pin's connected to
			for (UEdGraphPin*& OtherPin : TargetPin->LinkedTo)
			{
				if (OtherPin)
				{
					UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
					if (OtherNode)
					{
						NodeList.Add(OtherNode);
					}
				}
			}

			BreakPinLinks(*TargetPin, false);
		}
	}

	// Send all nodes that lost connections a notification
	for (auto It = NodeList.CreateConstIterator(); It; ++It)
	{
		UEdGraphNode* Node = (*It);
		Node->NodeConnectionListChanged();
	}

	TargetNode.GetGraph()->NotifyGraphChanged();
	Cast<UQuestTreeEdGraph>(TargetNode.GetGraph())->MarkCompileStateDirty();
}

void UQuestTreeEdGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(*FQuestTreeEditorCommon::GraphSchemaActions, LOCTEXT("QuestTreeSchemaBreakingPinLinks", "Break Pin Links"), nullptr);
	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	// Compile graph nodes if need to send notifications.
	if (bSendsNodeNotifcation)
	{
		TargetPin.GetOwningNode()->GetGraph()->NotifyGraphChanged();
		Cast<UQuestTreeEdGraph>(TargetPin.GetOwningNode()->GetGraph())->MarkCompileStateDirty();
	}
}

void UQuestTreeEdGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(*FQuestTreeEditorCommon::GraphSchemaActions, LOCTEXT("QuestTreeSchemaBreakSinglePinLink", "Break Single Pin Link"), nullptr);
	Super::BreakSinglePinLink(SourcePin, TargetPin);

	SourcePin->GetOwningNode()->GetGraph()->NotifyGraphChanged();
	Cast<UQuestTreeEdGraph>(SourcePin->GetOwningNode()->GetGraph())->MarkCompileStateDirty();
}

void UQuestTreeEdGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	const UEdGraph* CurrentGraph = Context->Graph;
	const UEdGraphNode* InGraphNode = Context->Node;
	const UEdGraphPin* InGraphPin = Context->Pin;

	if (InGraphNode)
	{
		FToolMenuSection& Section = Menu->AddSection("EdGraphSchemaNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
		// Node contextual actions
		Section.AddMenuEntry(FGenericCommands::Get().Delete);
		Section.AddMenuEntry(FGenericCommands::Get().Cut);
		Section.AddMenuEntry(FGenericCommands::Get().Copy);
		Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
		Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);

		if (GetSelectedNodes(CurrentGraph).Num() > 1)
		{
			// Node Alignment actions
			Section.AddSubMenu("Alignment", LOCTEXT("AlignmentHeader", "Alignment"), FText(), FNewToolMenuDelegate::CreateLambda([](UToolMenu* AlignmentMenu)
				{
					{
						FToolMenuSection& InSection = AlignmentMenu->AddSection("EdGraphSchemaAlignment", LOCTEXT("AlignHeader", "Align"));
						InSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesTop);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesMiddle);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesBottom);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesLeft);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesCenter);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesRight);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().StraightenConnections);
					}

					{
						FToolMenuSection& InSection = AlignmentMenu->AddSection("EdGraphSchemaDistribution", LOCTEXT("DistributionHeader", "Distribution"));
						InSection.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesHorizontally);
						InSection.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesVertically);
					}
				}));
		}
	}
}

FConnectionDrawingPolicy* UQuestTreeEdGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraph) const
{
	return new FQuestTreeConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraph);
}

TSharedPtr<FQuestTreeEditor> UQuestTreeEdGraphSchema::GetQuestTreeGraphEditor(const UEdGraph* InGraph)
{
	if (InGraph)
	{
		const UQuestTreeEdGraph* QuestTreeEdGraph = Cast<UQuestTreeEdGraph>(InGraph);

		if (QuestTreeEdGraph && QuestTreeEdGraph->GetQuestTreeGraph())
		{
			TSharedPtr<IToolkit> FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(QuestTreeEdGraph->GetQuestTreeGraph());
			return StaticCastSharedPtr<FQuestTreeEditor>(FoundAssetEditor);
		}
	}

	return nullptr;
}

TArray<UQuestTreeEdGraphNode*> UQuestTreeEdGraphSchema::GetSelectedNodes(const UEdGraph* InGraph) const
{
	if (!InGraph)
	{
		return TArray<UQuestTreeEdGraphNode*>();
	}

	const TSharedPtr<FQuestTreeEditor> QuestTreeEditor = GetQuestTreeGraphEditor(InGraph);
	return QuestTreeEditor->GetSelectedNodes();
}

void UQuestTreeEdGraphSchema::InitNodeClasses()
{
	if (bQuestTreeNodeClassesInitialized)
	{
		return;
	}
	QuestTreeNodeClasses.Empty();

	// Construct list of non-abstract QuestTree node classes.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UQuestTreeNode::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable | CLASS_Deprecated))
		{
			if (const UQuestTreeNode* QuestTreeNode = It->GetDefaultObject<UQuestTreeNode>())
			{
				QuestTreeNodeClasses.Add(*It);
			}
		}
	}

	QuestTreeNodeClasses.Sort();
	bQuestTreeNodeClassesInitialized = true;
}

void UQuestTreeEdGraphSchema::GetQuestTreeNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph) const
{
	InitNodeClasses();
	const UEdGraphPin* FromPin = ActionMenuBuilder.FromPin;

	for (UClass*& NodeClass : QuestTreeNodeClasses)
	{
		UQuestTreeNode* DefaultNode = NodeClass->GetDefaultObject<UQuestTreeNode>();
		if (!FromPin)
		{
			FText ActionCategory = GetClassCategory(NodeClass);
			if (ActionCategory.IsEmpty())
			{
				UClass* Class = NodeClass->GetSuperClass();
				while (Class && ActionCategory.IsEmpty())
				{
					ActionCategory = GetClassCategory(Class);
					Class = Class->GetSuperClass();
				}
			}

			int32 Priority = 5;

			FText Name = FText::FromString(NodeClass->GetDescription());
			FText AddToolTip = NodeClass->GetToolTipText();
			FText Keywords = NodeClass->GetMetaDataText(TEXT("Keywords"), TEXT("UObjectKeywords"), GetClass()->GetFullGroupName(false));

			// Create normal node.
			TSharedPtr<FQuestTreeGraphSchemaAction_NewNode> NewNodeAction(
				new FQuestTreeGraphSchemaAction_NewNode(
					ActionCategory,
					Name,
					AddToolTip,
					Priority,
					Keywords));
			NewNodeAction->NodeClass = NodeClass;
			ActionMenuBuilder.AddAction(NewNodeAction);
		}
	}
}

#undef LOCTEXT_NAMESPACE