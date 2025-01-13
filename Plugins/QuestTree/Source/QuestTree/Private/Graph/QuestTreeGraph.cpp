// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeGraph.h"
#include "Graph/QuestTreeNode.h"
#include "Graph/QuestTreeNodePin.h"
#include "Components/QuestTreeManagerComponent.h"

#define LOCTEXT_NAMESPACE "QuestTreeGraph"

void UQuestTreeGraph::ExecuteQuestTree(UQuestTreeManagerComponent* InQuestManager)
{
	SetupQuestTreeNodes();
	RootNode->GetOutputPins()[0]->ExecutePin(InQuestManager);
}

#if WITH_EDITOR
void UQuestTreeGraph::SetEdGraph(UEdGraph* InGraph)
{
	EdGraph = InGraph;
}

UQuestTreeNode* UQuestTreeGraph::ConstructNewNode(TSubclassOf<UQuestTreeNode> NewNodeClass, const FVector2D& Position, bool bSelectNewNode)
{
	check(NewNodeClass);
	Modify();
	EdGraph->Modify();

	UQuestTreeNode* NewNode = NewObject<UQuestTreeNode>(this, NewNodeClass, NAME_None, RF_Transactional);
	MarkPackageDirty();
	NewNode->SetGraph(this);
	NewNode->SetPosition(Position);
	SetCompileStatus(EQuestTreeCompileStatus::Uncompiled);
	
	return NewNode;
}

void UQuestTreeGraph::RemoveNode(UQuestTreeNode* InNode)
{
	check(InNode);
	
	InNode->Modify();
	InNode->PreDeleteNode();
	InNode->MarkAsGarbage();
	Modify();
	MarkPackageDirty();
}

void UQuestTreeGraph::PostCopyNode(UQuestTreeNode* InNode)
{
	InNode->Rename(nullptr, this, REN_DontCreateRedirectors);
}
#endif // WITH_EDITOR

TArray<FQuestTreeData> UQuestTreeGraph::GetAllQuestsDataFromQuestTree() const
{
	TArray<FQuestTreeData> OutData;
	for (TObjectPtr<UQuestTreeNode> Node : AllNodes)
	{
		if (UQuestTreeNode_Quest* AsQuestNode = Cast<UQuestTreeNode_Quest>(Node.Get()))
		{
			OutData.Add(AsQuestNode->QuestData);
		}
	}
	return OutData;
}

FGameplayTagContainer UQuestTreeGraph::GetAllQuestsTagsFromQuestTree() const
{
	FGameplayTagContainer OutData;
	for (TObjectPtr<UQuestTreeNode> Node : AllNodes)
	{
		if (UQuestTreeNode_Quest* AsQuestNode = Cast<UQuestTreeNode_Quest>(Node.Get()))
		{
			OutData.AddTag(AsQuestNode->QuestData.QuestTag);
		}
	}
	return OutData;
}

bool UQuestTreeGraph::FindQuestDataByTag(const FGameplayTag& InQuestTag, FQuestTreeData& FoundQuestData) const
{
	FoundQuestData = FQuestTreeData();
	if (!InQuestTag.IsValid())
		return false;

	for (const FQuestTreeData QuestData : GetAllQuestsDataFromQuestTree())
	{
		if (QuestData.QuestTag == InQuestTag)
		{
			FoundQuestData = QuestData;
			return true;
		}
	}
	return false;
}

void UQuestTreeGraph::SetupQuestTreeNodes()
{
	for (TObjectPtr<UQuestTreeNode> Node : AllNodes)
	{
		Node->SetupNode();
	}
}

#undef LOCTEXT_NAMESPACE