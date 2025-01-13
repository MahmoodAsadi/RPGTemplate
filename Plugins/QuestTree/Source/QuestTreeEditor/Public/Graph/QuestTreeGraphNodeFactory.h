// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

class SGraphNode;
class UEdGraphNode;

/**
 * 
 */
class FQuestTreeGraphNodeFactory : public FGraphPanelNodeFactory
{
public:

	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override;

};