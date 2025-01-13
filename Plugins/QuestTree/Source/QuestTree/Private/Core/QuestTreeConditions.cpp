// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/QuestTreeConditions.h"
#include "Components/QuestTreeManagerComponent.h"
#include "Core/QuestTreeSubsystem.h"

#if WITH_EDITOR
FString UQuestTreeConditionBase::GetDescription() const
{
	FString ClassTitle = GetClass()->GetDisplayNameText().ToString();
	ClassTitle.Append(":\n");
	return ClassTitle;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_AnyConditionsMatch::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	for (UQuestTreeConditionBase* Condition : Conditions)
	{
		if (Condition && Condition->PerformQuestConditionCheck(InQuestManager))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
FString UQuestTreeCondition_AnyConditionsMatch::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	for (UQuestTreeConditionBase* Condition : Conditions)
	{
		if (!Condition)
			continue;

		FinalDescription.Append(Condition->GetDescription());
		FinalDescription.Append("\n");
	}

	return FinalDescription;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_AllConditionsMatch::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	for (UQuestTreeConditionBase* Condition : Conditions)
	{
		if (!Condition || !Condition->PerformQuestConditionCheck(InQuestManager))
		{
			return false;
		}
	}

	return true;
}

#if WITH_EDITOR
FString UQuestTreeCondition_AllConditionsMatch::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	for (UQuestTreeConditionBase* Condition : Conditions)
	{
		if (!Condition)
			continue;

		FinalDescription.Append(Condition->GetDescription());
		FinalDescription.Append("\n");
	}

	return FinalDescription;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_NoConditionsMatch::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	for (UQuestTreeConditionBase* Condition : Conditions)
	{
		if (Condition && Condition->PerformQuestConditionCheck(InQuestManager))
		{
			return false;
		}
	}

	return true;
}

#if WITH_EDITOR
FString UQuestTreeCondition_NoConditionsMatch::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	for (UQuestTreeConditionBase* Condition : Conditions)
	{
		if (!Condition)
			continue;

		FinalDescription.Append(Condition->GetDescription());
		FinalDescription.Append("\n");
	}

	return FinalDescription;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_ActiveQuest::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	return InQuestManager->DoesActiveQuestsMatchesQuery(ActiveQuestQuery);
}

#if WITH_EDITOR
FString UQuestTreeCondition_ActiveQuest::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	FinalDescription.Append(ActiveQuestQuery.GetDescription());
	return FinalDescription;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_CompletedQuest::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	return InQuestManager->DoesCompletedQuestsMatchesQuery(CompletedQuestQuery);
}

#if WITH_EDITOR
FString UQuestTreeCondition_CompletedQuest::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	FinalDescription.Append(CompletedQuestQuery.GetDescription());
	return FinalDescription;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_ExpiredQuest::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	return InQuestManager->DoesExpiredQuestsMatchesQuery(ExpiredQuestQuery);
}

#if WITH_EDITOR
FString UQuestTreeCondition_ExpiredQuest::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	FinalDescription.Append(ExpiredQuestQuery.GetDescription());
	return FinalDescription;
}
#endif // WITH_EDITOR

bool UQuestTreeCondition_ExecutedEvent::PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const
{
	return InQuestManager->DoesExecutedEventsMatchesQuery(ExecutedEventQuery);
}

#if WITH_EDITOR
FString UQuestTreeCondition_ExecutedEvent::GetDescription() const
{
	FString FinalDescription = Super::GetDescription();
	FinalDescription.Append(ExecutedEventQuery.GetDescription());
	return FinalDescription;
}
#endif // WITH_EDITOR