// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/QuestTreeHelper.h"
#include "QuestTreeGraph.generated.h"

class UEdGraph;
class UQuestTreeNode;
class UQuestTreeManagerComponent;
class UQuestTreeGiverComponent;

/**
 * 
 */
UCLASS(BlueprintType)
class QUESTTREE_API UQuestTreeGraph : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Quest Tree")
	void ExecuteQuestTree(UQuestTreeManagerComponent* InQuestManager);

#if WITH_EDITOR
	void SetEdGraph(UEdGraph* InGraph);
	UEdGraph* GetEdGraph() const { return EdGraph; }

	void SetCompileStatus(EQuestTreeCompileStatus NewStatus) { CompileStatus = NewStatus; }
	EQuestTreeCompileStatus GetCompileStatus() { return CompileStatus; }

	UQuestTreeNode* ConstructNewNode(TSubclassOf<UQuestTreeNode> NewNodeClass, const FVector2D& Position, bool bSelectNewNode = true);

	UFUNCTION()
	void RemoveNode(UQuestTreeNode* InNode);

	UFUNCTION()
	void SetRootNode(UQuestTreeNode_Root* InNode) { RootNode = InNode; }

	UFUNCTION()
	void PostCopyNode(UQuestTreeNode* InNode);

	UFUNCTION()
	void RegisterGraphNodes(TArray<UQuestTreeNode*> InQuestTreeNodes) { AllNodes = InQuestTreeNodes; }

#endif // WITH_EDITOR

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Graph")
	TArray<FQuestTreeData> GetAllQuestsDataFromQuestTree() const;

	UFUNCTION(BlueprintPure, Category = "Quest Tree|Quest Tree Graph")
	FGameplayTagContainer GetAllQuestsTagsFromQuestTree() const;

	UFUNCTION(BlueprintCallable, Category = "Quest Tree|Quest Tree Graph", meta = (AutoCreateRefTerm = "InQuestTag"))
	bool FindQuestDataByTag(const FGameplayTag& InQuestTag, FQuestTreeData& FoundQuestData) const;

private:

	UFUNCTION()
	void SetupQuestTreeNodes();

#if WITH_EDITORONLY_DATA

	/** The editor graph associated with this Quest Graph */
	UPROPERTY()
	UEdGraph* EdGraph = nullptr;

	UPROPERTY()
	EQuestTreeCompileStatus CompileStatus;

#endif

	UPROPERTY()
	TArray<TObjectPtr<UQuestTreeNode>> AllNodes;

	UPROPERTY()
	UQuestTreeNode_Root* RootNode;
	
};