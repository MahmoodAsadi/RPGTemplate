// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeEdGraph.h"
#include "Graph/QuestTreeEdGraphNode.h"
#include "Graph/QuestTreeGraph.h"
#include "Graph/QuestTreeNode.h"
#include "Graph/QuestTreeNodePin.h"


UQuestTreeEdGraph::UQuestTreeEdGraph(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Setup handler for changing the graph
	// TODOMAHMOOD : Needs this? bind to necessary delegate.
	//AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateUObject(this, &UDialogueEdGraph::OnDialogueGraphChanged));
}

bool UQuestTreeEdGraph::Modify(bool bAlwaysMarkDirty)
{
	bool ModifyReturnValue = Super::Modify(bAlwaysMarkDirty);
	GetQuestTreeGraph()->Modify();

	for (UEdGraphNode* Node : Nodes)
	{
		Node->Modify();
	}

	return ModifyReturnValue;
}

void UQuestTreeEdGraph::PostEditUndo()
{
	Super::PostEditUndo();
	NotifyGraphChanged();
}

void UQuestTreeEdGraph::PostInitProperties()
{
	Super::PostInitProperties();
	
	//Set up speaker roles changed event
	UQuestTreeGraph* OuterQuestGraph = Cast<UQuestTreeGraph>(GetOuter());
	if (OuterQuestGraph)
	{
		// TODOMAHMOOD : Needs this? bind to necessary delegate.
		//OuterDialogue->OnSpeakerRolesChanged.BindUFunction(this, "OnSpeakerRolesChanged");
	}
}

void UQuestTreeEdGraph::NotifyGraphChanged()
{
	Super::NotifyGraphChanged();
	Modify();
}

void UQuestTreeEdGraph::MarkCompileStateDirty()
{
	if (QuestGraph)
	{
		QuestGraph->SetCompileStatus(EQuestTreeCompileStatus::Uncompiled);
	}
}

void UQuestTreeEdGraph::ConstructEdGraphNode(UQuestTreeNode* InGraphNode)
{
	check(InGraphNode);
	Modify();
	FGraphNodeCreator<UQuestTreeEdGraphNode> GraphNodeCreator(*this);
	UQuestTreeEdGraphNode* EdGraphNode = GraphNodeCreator.CreateNode(false);
	EdGraphNode->Construct(InGraphNode);
	GraphNodeCreator.Finalize();
	NotifyGraphChanged();
}

void UQuestTreeEdGraph::SetQuestTreeGraph(UQuestTreeGraph* InGraph)
{
	QuestGraph = InGraph;
}

UQuestTreeGraph* UQuestTreeEdGraph::GetQuestTreeGraph() const
{
	return CastChecked<UQuestTreeGraph>(GetOuter());
}

EQuestTreeCompileStatus UQuestTreeEdGraph::CompileGraph(TArray<FQuestTreeCompileErrorInfo>& OutIssues)
{
	if (!ensure(GraphRootNode))
		return EQuestTreeCompileStatus::Failed;

	if (!ensure(GraphRootNode->Pins.IsValidIndex(0)))
		return EQuestTreeCompileStatus::Failed;

	UEdGraphPin* RootPin = GraphRootNode->Pins[0];
	UQuestTreeNode* QuestRootNode = GraphRootNode->GetQuestTreeNode();
	UQuestTreeNodePin* RootNodePin = QuestRootNode->GetPinByName(RootPin->PinName, EGPD_Output);
	if (!ensure(RootNodePin))
		return EQuestTreeCompileStatus::Failed;

	Modify();
	QuestGraph->Modify();

	TArray<UQuestTreeNode*> AllQuestTreeNodes;
	// Reset all pin connections, will repopulate them after.
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestTreeEdGraphNode* QuestTreeEdGraphNode = Cast<UQuestTreeEdGraphNode>(Node))
		{
			QuestTreeEdGraphNode->RestPinConnections();

			if (QuestTreeEdGraphNode != GraphRootNode && QuestTreeEdGraphNode->IsConnectedToGraphRoot())
			{
				AllQuestTreeNodes.Add(QuestTreeEdGraphNode->GetQuestTreeNode());
			}
		}
	}

	QuestGraph->RegisterGraphNodes(AllQuestTreeNodes);
	TArray<UEdGraphPin*> VisitedPins;
	GraphRootNode->PopulateNodePinConnections(VisitedPins);

	TArray<const UEdGraphNode*> VisitedNodes;
	TArray<FQuestTreeCompileErrorInfo> FoundIssues;
	GraphRootNode->CheckGraphNodeCompileStatusRecursively(VisitedNodes, FoundIssues);
	OutIssues = FoundIssues;
	
	return FindGraphCompileStatusFromFoundIssues(FoundIssues);
}

EQuestTreeCompileStatus UQuestTreeEdGraph::FindGraphCompileStatusFromFoundIssues(const TArray<FQuestTreeCompileErrorInfo>& FoundIssues)
{
	if (FoundIssues.Num() == 0)
		return EQuestTreeCompileStatus::Compiled;

	for (const FQuestTreeCompileErrorInfo& Issue : FoundIssues)
	{
		if (Issue.Status == EQuestTreeCompileStatus::Failed)
			return EQuestTreeCompileStatus::Failed;
	}

	return EQuestTreeCompileStatus::Warning;
}