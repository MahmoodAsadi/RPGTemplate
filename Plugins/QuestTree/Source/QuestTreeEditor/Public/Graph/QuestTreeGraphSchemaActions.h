// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"
#include "QuestTreeGraphSchemaActions.generated.h"

class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;
class UQuestTreeNode;

USTRUCT()
struct FQuestTreeGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_BODY()

public:

	// Inherit the base class's constructors
	using FEdGraphSchemaAction::FEdGraphSchemaAction;

	// Simple type info
	static FName StaticGetTypeId()
	{
		static FName Type("FQuestTreeGraphSchemaAction_NewNode");
		return Type;
	}

	UPROPERTY()
	TSubclassOf<UQuestTreeNode> NodeClass;

	// FEdGraphSchemaAction interface
	virtual FName GetTypeId() const override { return StaticGetTypeId(); }
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	// End of FEdGraphSchemaAction interface
};