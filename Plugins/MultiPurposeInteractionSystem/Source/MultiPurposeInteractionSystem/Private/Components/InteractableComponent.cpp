// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InteractableComponent.h"
#include "Components/InteractionManagerComponent.h"


UInteractableComponent::UInteractableComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetAutoActivate(true);
	OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::OnBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::OnEndOverlap);
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	GatherExternalCollisions();
}

void UInteractableComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	TSet<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor))
		{
			if (UInteractionManagerComponent* InteractionManager = Actor->GetComponentByClass<UInteractionManagerComponent>())
			{
				InteractionManager->RegisterInteractable(this);
			}
		}
	}
}

void UInteractableComponent::Deactivate()
{
	TSet<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor))
		{
			if (UInteractionManagerComponent* InteractionManager = Actor->GetComponentByClass<UInteractionManagerComponent>())
			{
				InteractionManager->UnregisterInteractable(this);
			}
		}
	}

	Super::Deactivate();
}

void UInteractableComponent::SetInteractionEnable(bool bEnable)
{
	if (InteractionVisualPresentationInfo.UsesOutline())
	{
		if (!InteractionVisualPresentationInfo.ComponentTag.IsNone())
		{
			TArray<UActorComponent*> OtherComponents = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(), InteractionVisualPresentationInfo.ComponentTag);
			for (UActorComponent* Component : OtherComponents)
			{
				if (UPrimitiveComponent* AsPrimitiveComponent = Cast<UPrimitiveComponent>(Component))
				{
					AsPrimitiveComponent->SetRenderCustomDepth(bEnable);
				}
			}
		}
	}

	OnInteractionActivate.Broadcast(this, bEnable);
}

void UInteractableComponent::Interact(UObject* Instigator)
{
	OnInteractableInteracted.Broadcast(this, Instigator);
}

void UInteractableComponent::GatherExternalCollisions()
{
	for (const FName& Tag : ExternalCollisionTags)
	{
		TArray<UActorComponent*> OtherComponents = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(), Tag);
		for (UActorComponent* Component : OtherComponents)
		{
			if (UPrimitiveComponent* AsPrimitiveComponent = Cast<UPrimitiveComponent>(Component))
			{
				if (!ExternalComponents.Contains(AsPrimitiveComponent))
				{
					ExternalComponents.AddUnique(AsPrimitiveComponent);
					AsPrimitiveComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UInteractableComponent::OnBeginOverlap);
					AsPrimitiveComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UInteractableComponent::OnEndOverlap);
				}
			}
		}
	}
}

void UInteractableComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsActive())
		return;
	
	UInteractionManagerComponent* InteractionManager = OtherActor->GetComponentByClass<UInteractionManagerComponent>();
	if (IsValid(InteractionManager))
	{
		InteractionManager->RegisterInteractable(this);
	}
}

void UInteractableComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsActive())
		return;

	UInteractionManagerComponent* InteractionManager = OtherActor->GetComponentByClass<UInteractionManagerComponent>();
	if (!IsValid(InteractionManager))
		return;
	
	if (IsOverlappingActor(OtherActor))
		return;

	bool bStillOverlapping = false;
	for (UPrimitiveComponent* Component : ExternalComponents)
	{
		if (Component->IsOverlappingActor(OtherActor))
		{
			bStillOverlapping = true;
		}
	}

	if (!bStillOverlapping)
	{
		InteractionManager->UnregisterInteractable(this);
	}
}