// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeNode.h"
#include "Graph/QuestTreeNodePin.h"
#include "Core/QuestTreeConditions.h"
#include "Core/QuestTreeEvents.h"
#include "Core/QuestTreeSubsystem.h"
#include "Core/QuestTreeObjectives.h"
#include "Components/QuestTreeManagerComponent.h"

#define LOCTEXT_NAMESPACE "QuestTreeNode"


UQuestTreeNode::UQuestTreeNode(const FObjectInitializer& ObjectInitializer)
{
#if WITH_EDITOR
	NodeTitle = GetClass()->GetDisplayNameText().ToString();
#endif // WITH_EDITOR
}

#if WITH_EDITOR
FText UQuestTreeNode::GetNodeTitle() const
{
	if (bUseCustomTitle && !NodeTitle.IsEmpty())
		return FText::FromString(NodeTitle);

	return GetClass()->GetDisplayNameText();
}

const FSlateBrush* UQuestTreeNode::GetNodeIcon() const
{
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Root.Icon"));
}

void UQuestTreeNode::MakeInputPinsMaker(const TArray<FQuestTreePinMaker>& InPinsInfo)
{
	InputPinsMakers = InPinsInfo;
}

void UQuestTreeNode::MakeOutputPinsMaker(const TArray<FQuestTreePinMaker>& InPinsInfo)
{
	OutputPinsMakers = InPinsInfo;
}

void UQuestTreeNode::AddInputPinMaker(const FQuestTreePinMaker& InPinInfo)
{
	InputPinsMakers.Add(InPinInfo);
}

void UQuestTreeNode::AddOutputPinMaker(const FQuestTreePinMaker& InPinInfo)
{
	OutputPinsMakers.Add(InPinInfo);
}

void UQuestTreeNode::ResetPinConnections()
{
	for (UQuestTreeNodePin* Pin : InputPins)
	{
		Pin->ConnectedPins.Reset();
	}

	for (UQuestTreeNodePin* Pin : OutputPins)
	{
		Pin->ConnectedPins.Reset();
	}
}
#endif // WITH_EDITOR

UQuestTreeNodePin* UQuestTreeNode::GetPinByName(const FName& InName) const
{
	for (UQuestTreeNodePin* Pin : InputPins)
	{
		if (Pin->EdGraphPinName == InName)
		{
			return Pin;
		}
	}

	for (UQuestTreeNodePin* Pin : OutputPins)
	{
		if (Pin->EdGraphPinName == InName)
		{
			return Pin;
		}
	}

	return nullptr;
}

UQuestTreeNodePin* UQuestTreeNode::GetPinByName(const FName& InName, TEnumAsByte<enum EEdGraphPinDirection> InDirection) const
{
	switch (InDirection)
	{
		case EEdGraphPinDirection::EGPD_Input:

			for (UQuestTreeNodePin* Pin : InputPins)
			{
				if (Pin->EdGraphPinName == InName)
				{
					return Pin;
				}
			}

		break;

		case EEdGraphPinDirection::EGPD_Output:

			for (UQuestTreeNodePin* Pin : OutputPins)
			{
				if (Pin->EdGraphPinName == InName)
				{
					return Pin;
				}
			}

		break;

	default:
		return nullptr;
	}
	return nullptr;
}

void UQuestTreeNode::ExecutePin(const FName& InName, UQuestTreeManagerComponent* InQuestManager)
{
	if (UQuestTreeNodePin* Pin = GetPinByName(InName, EGPD_Output))
	{
		Pin->ExecutePin(InQuestManager);
	}
}

UQuestTreeNode_Root::UQuestTreeNode_Root(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	MakeOutputPinsMaker({ FQuestTreePinMaker("In", "Graph Root") });
#endif // WITH_EDITOR
}

UQuestTreeNode_Quest::UQuestTreeNode_Quest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	AddInputPinMaker(FQuestTreePinMaker("InExecute", "Make quest available if conditions met (Automatically starts if bAutoStart is true)"));
	AddInputPinMaker(FQuestTreePinMaker("InExpire", "Expire quest", FLinearColor::Red));
	AddOutputPinMaker(FQuestTreePinMaker("OutAvailable", "On quest available"));
	AddOutputPinMaker(FQuestTreePinMaker("OutInProgress", "On quest started"));
	AddOutputPinMaker(FQuestTreePinMaker("OutCompleted", "On quest complete", FLinearColor::Green));
	AddOutputPinMaker(FQuestTreePinMaker("OutFailed", "On quest failed", FLinearColor::Red));
	AddOutputPinMaker(FQuestTreePinMaker("OutExpired", "On quest expired", FLinearColor::Red));
#endif // WITH_EDITOR
}

void UQuestTreeNode_Quest::SetupNode()
{
	GetPinByName("InExecute")->OnExecutePin.AddUniqueDynamic(this, &UQuestTreeNode_Quest::OnExecuteInExecutionPin);
	GetPinByName("InExpire")->OnExecutePin.AddUniqueDynamic(this, &UQuestTreeNode_Quest::OnExecuteInExpirePin);
}

void UQuestTreeNode_Quest::OnExecuteInExecutionPin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager)
{
	switch (InQuestManager->GetQuestStatus(QuestData.QuestTag))
	{
		case EQuestTreeStatus::None:

			if (!Condition || Condition->PerformQuestConditionCheck(InQuestManager))
			{
				InQuestManager->AddAvailableQuest(QuestData, false);
				ExecutePin("OutAvailable", InQuestManager);
			}

			break;

		case EQuestTreeStatus::Available:

			ExecutePin("OutAvailable", InQuestManager);
			break;

		case EQuestTreeStatus::InProgress:
			ExecutePin("OutInProgress", InQuestManager);
			break;

		case EQuestTreeStatus::Completed:
			ExecutePin("OutCompleted", InQuestManager);
			break;

		case EQuestTreeStatus::Failed:
			ExecutePin("OutFailed", InQuestManager);
			break;

		case EQuestTreeStatus::Expired:
			ExecutePin("OutExpired", InQuestManager);
			break;

	default:
		break;
	}
}

void UQuestTreeNode_Quest::OnExecuteInExpirePin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager)
{
	InQuestManager->ExpireQuest(QuestData.QuestTag, false);
	ExecutePin("OutExpired", InQuestManager);
}

#if WITH_EDITOR
FText UQuestTreeNode_Quest::GetNodeTitle() const
{
	if (bUseCustomTitle && !NodeTitle.IsEmpty())
		return FText::FromString(NodeTitle);

	if (!QuestData.Title.IsEmpty())
		return FText::Format(LOCTEXT("QuestTitleTextLabel", "{0}: {1}"), Super::GetNodeTitle(), FText::FromString(QuestData.Title.ToString()));

	return Super::GetNodeTitle();
}

const FSlateBrush* UQuestTreeNode_Quest::GetNodeIcon() const
{
	const EQuestTreeType QuestType = QuestData.QuestType;
	
	switch (QuestType)
	{
		case EQuestTreeType::MainQuest:
			return FAppStyle::GetBrush(TEXT("ContentBrowser.PopupMessageIcon"));

		case EQuestTreeType::SideQuest:
			return FAppStyle::GetBrush(TEXT("GraphEditor.Macro.IsValid_16x"));

		case EQuestTreeType::DailyQuest:
			return FAppStyle::GetBrush(TEXT("GraphEditor.Timeline_16x"));

	default:
		return Super::GetNodeIcon();
	}

	return Super::GetNodeIcon();
}

FText UQuestTreeNode_Quest::GetNodeDetailText() const
{
	if (!bShowDescription)
		return FText();

	TArray<FText> PerDetailText;
	FText CombinedText;
	if (QuestData.QuestTag.IsValid())
	{
		PerDetailText.Add(FText::Format(LOCTEXT("QuestTagTextLabel", "QuestTag:\n{0}"), FText::FromString(QuestData.QuestTag.ToString())));
		PerDetailText.Add(FText::Format(LOCTEXT("QuestAutoStartLabel", "Auto Start: {0}"), QuestData.bAutoStart ? FText::FromString("Enabled") : FText::FromString("Disabled")));
		PerDetailText.Add(FText::Format(LOCTEXT("QuestNumberOfObjectsLabel", "Objective Count: {0}"), FText::AsNumber(QuestData.Objectives.Num())));
		PerDetailText.Add(FText::Format(LOCTEXT("QuestHasConditionLabel", "Has Condition: {0}"), IsValid(Condition) ? FText::FromString("True") : FText::FromString("Fale")));
		
		if (!QuestData.Brief.IsEmpty())
			PerDetailText.Add(FText::Format(LOCTEXT("QuestBriefTextLabel", "Brief:\n{0}"), FText::FromString(QuestData.Brief.ToString())));

		FString StringCombined;
		for (FText DetailText : PerDetailText)
		{
			if (StringCombined.IsEmpty())
			{
				StringCombined = DetailText.ToString();
			}
			else
			{
				StringCombined.Append("\n");
				StringCombined.Append(DetailText.ToString());
			}
		}
		CombinedText = FText::FromString(StringCombined);
	}

	return CombinedText;
}

void UQuestTreeNode_Quest::CheckNodeCompileStatus(TArray<FQuestTreeCompileErrorInfo>& FoundIssues)
{
	Super::CheckNodeCompileStatus(FoundIssues);

	if (!QuestData.QuestTag.IsValid())
	{
		FQuestTreeCompileErrorInfo CompileInfo;
		CompileInfo.OwningNode = GetGraphNode();
		CompileInfo.Status = EQuestTreeCompileStatus::Failed;
		CompileInfo.IssueDescription = FText::Format(LOCTEXT("CompileDescriptionLabel", "[{0}]: Has invalid quest tag"), GetNodeTitle());
		FoundIssues.Add(CompileInfo);
		CachedNodeCompileStatus.Add(CompileInfo);
	}

	if (QuestData.Title.IsEmpty())
	{
		FQuestTreeCompileErrorInfo CompileInfo;
		CompileInfo.OwningNode = GetGraphNode();
		CompileInfo.Status = EQuestTreeCompileStatus::Warning;
		CompileInfo.IssueDescription = FText::Format(LOCTEXT("CompileDescriptionLabel", "[{0}]: Missing quest Title"), GetNodeTitle());
		FoundIssues.Add(CompileInfo);
		CachedNodeCompileStatus.Add(CompileInfo);
	}

	if (QuestData.Brief.IsEmpty())
	{
		FQuestTreeCompileErrorInfo CompileInfo;
		CompileInfo.OwningNode = GetGraphNode();
		CompileInfo.Status = EQuestTreeCompileStatus::Warning;
		CompileInfo.IssueDescription = FText::Format(LOCTEXT("CompileDescriptionLabel", "[{0}]: Missing quest Brief"), GetNodeTitle());
		FoundIssues.Add(CompileInfo);
		CachedNodeCompileStatus.Add(CompileInfo);
	}

	if (QuestData.Objectives.Num() == 0)
	{
		FQuestTreeCompileErrorInfo CompileInfo;
		CompileInfo.OwningNode = GetGraphNode();
		CompileInfo.Status = EQuestTreeCompileStatus::Warning;
		CompileInfo.IssueDescription = FText::Format(LOCTEXT("CompileDescriptionLabel", "[{0}]: Quest has no objective"), GetNodeTitle());
		FoundIssues.Add(CompileInfo);
		CachedNodeCompileStatus.Add(CompileInfo);
	}
	else
	{
		for (int32 i = 0; i < QuestData.Objectives.Num(); i++)
		{
			if (!IsValid(QuestData.Objectives[i]))
			{
				FQuestTreeCompileErrorInfo CompileInfo;
				CompileInfo.OwningNode = GetGraphNode();
				CompileInfo.Status = EQuestTreeCompileStatus::Failed;
				CompileInfo.IssueDescription = FText::Format(LOCTEXT("QuestCompileInfoLabel", "[{0}]: Quest has invalid objective at index [{1}]"), GetNodeTitle(), FText::AsNumber(i));
				FoundIssues.Add(CompileInfo);
				CachedNodeCompileStatus.Add(CompileInfo);
			}
			else
			{
				if (!QuestData.Objectives[i]->ObjectiveTag.IsValid())
				{
					FQuestTreeCompileErrorInfo CompileInfo;
					CompileInfo.OwningNode = GetGraphNode();
					CompileInfo.Status = EQuestTreeCompileStatus::Failed;
					CompileInfo.IssueDescription = FText::Format(LOCTEXT("QuestCompileInfoLabel", "[{0}]: Quest objective at index [{0}] has invalid tag"), GetNodeTitle(), FText::AsNumber(i));
					FoundIssues.Add(CompileInfo);
					CachedNodeCompileStatus.Add(CompileInfo);
				}

				if (QuestData.Objectives[i]->ObjectiveBrief.IsEmpty())
				{
					FQuestTreeCompileErrorInfo CompileInfo;
					CompileInfo.OwningNode = GetGraphNode();
					CompileInfo.Status = EQuestTreeCompileStatus::Warning;
					CompileInfo.IssueDescription = FText::Format(LOCTEXT("QuestCompileInfoLabel", "[{0}]: Quest objective at index [{0}] missing ObjectiveBrief"), GetNodeTitle(), FText::AsNumber(i));
					FoundIssues.Add(CompileInfo);
					CachedNodeCompileStatus.Add(CompileInfo);
				}
			}
		}
	}
}

void UQuestTreeNode_Quest::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (QuestData.Objectives.Num() > 0)
	{
		// Make sure last objective would have "bFailQuestIfUnsuccessful" set to be true.
		if (UQuestTreeObjective* LastObjective = QuestData.Objectives[QuestData.Objectives.Num() - 1])
		{
			LastObjective->bFailQuestIfUnsuccessful = true;
		}
	}
}
#endif // WITH_EDITOR

UQuestTreeNode_Branch::UQuestTreeNode_Branch(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	AddInputPinMaker(FQuestTreePinMaker("InExecute", "In Execution"));
	AddOutputPinMaker(FQuestTreePinMaker("OutTrue", "True", FLinearColor::Green));
	AddOutputPinMaker(FQuestTreePinMaker("OutFalse", "False", FLinearColor::Red));
#endif // WITH_EDITOR
}

void UQuestTreeNode_Branch::SetupNode()
{
	GetPinByName("InExecute")->OnExecutePin.AddUniqueDynamic(this, &UQuestTreeNode_Branch::OnExecuteInExecutionPin);
}

void UQuestTreeNode_Branch::OnExecuteInExecutionPin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager)
{
	const bool bConditionMatch = Condition->PerformQuestConditionCheck(InQuestManager);
	ExecutePin(bConditionMatch ? "OutTrue" : "OutFalse", InQuestManager);
}

#if WITH_EDITOR
const FSlateBrush* UQuestTreeNode_Branch::GetNodeIcon() const
{
	return FAppStyle::GetBrush(TEXT("GraphEditor.Branch_16x"));
}

FText UQuestTreeNode_Branch::GetNodeDetailText() const
{
	if (!bShowDescription)
		return FText();

	if (!IsValid(Condition))
		return FText();
	
	return FText::FromString(Condition->GetDescription());
}
#endif // WITH_EDITOR

UQuestTreeNode_Event::UQuestTreeNode_Event(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	AddInputPinMaker(FQuestTreePinMaker("InExecute", "In Execution"));
	AddOutputPinMaker(FQuestTreePinMaker("OutExecute", "Out Execute"));
#endif // WITH_EDITOR
}

void UQuestTreeNode_Event::SetupNode()
{
	GetPinByName("InExecute")->OnExecutePin.AddUniqueDynamic(this, &UQuestTreeNode_Event::OnExecuteInExecutionPin);
}

void UQuestTreeNode_Event::OnExecuteInExecutionPin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager)
{
	if (EventClass && !InQuestManager->IsEventExecuted(EventTag))
	{
		EventClass->GetDefaultObject<UQuestTreeExecutableEvent>()->ExecuteEvent(InQuestManager);
		InQuestManager->ExecuteQuestEvent(EventTag, false);
	}

	GetPinByName("OutExecute")->ExecutePin(InQuestManager);
}

#if WITH_EDITOR
FText UQuestTreeNode_Event::GetNodeDetailText() const
{
	if (!bShowDescription)
		return FText();

	if (EventTag.IsValid() && IsValid(EventClass))
	{
		return FText::Format(LOCTEXT("EventNodeTitleTextLabel", "EventTag: {0}\nEventClass: {1}"), 
			FText::FromString(EventTag.ToString()), EventClass->GetDisplayNameText());
	}

	return Super::GetNodeDetailText();
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE