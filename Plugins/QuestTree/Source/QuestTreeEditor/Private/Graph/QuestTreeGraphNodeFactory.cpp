// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeGraphNodeFactory.h"
#include "Graph/QuestTreeEdGraphNode.h"
#include "Graph/Slates/SQuestTreeGraphNode.h"


TSharedPtr<SGraphNode> FQuestTreeGraphNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	if (Cast<UQuestTreeEdGraphNode>(InNode))
	{
		return SNew(SQuestTreeGraphNode, Cast<UQuestTreeEdGraphNode>(InNode));
	}
	
	return nullptr;
}