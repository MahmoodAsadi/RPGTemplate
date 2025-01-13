// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeGraphSchemaActions.h"
#include "Graph/QuestTreeEdGraph.h"
#include "Graph/QuestTreeGraph.h"
#include "Graph/QuestTreeNode.h"
#include "Core/QuestTreeGraphHelper.h"

#define LOCTEXT_NAMESPACE "QuestTreeGraphSchemaActions"

UEdGraphNode* FQuestTreeGraphSchemaAction_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
    check(NodeClass);
    
	UQuestTreeEdGraph* EditorGraph = CastChecked<UQuestTreeEdGraph>(ParentGraph);
	UQuestTreeGraph* QuestTreeGraph = EditorGraph->GetQuestTreeGraph();

	const FScopedTransaction Transaction(*FQuestTreeEditorCommon::ContextIdentifier, LOCTEXT("FQuestTreeGraphSchemaActionNewNode", "QuestTree Editor: New Node"), nullptr);

	UQuestTreeNode* QuestNode = QuestTreeGraph->ConstructNewNode(NodeClass, Location, bSelectNewNode);
	EditorGraph->ConstructEdGraphNode(QuestNode);
	
	if (FromPin)
	{
		QuestNode->GetGraphNode()->AutowireNewNode(FromPin);
	}

	return QuestNode->GetGraphNode();
}

#undef LOCTEXT_NAMESPACE