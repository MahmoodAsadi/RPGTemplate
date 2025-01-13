// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InteractionManagerComponent.h"
#include "Components/InteractionWidgetComponent.h"
#include "Components/InteractableComponent.h"
#include "UI/InteractionUserWidget.h"

#include "Engine/AssetManager.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UInteractionManagerComponent::UInteractionManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	InteractionSourceDirectionInfo.DirectionType = EInteractionDirectionSource::Camera;
	InteractionSourceLocationInfo.LocationType = EInteractionLocationSource::Camera;
	LineOfSightSourceLocationInfo.LocationType = EInteractionLocationSource::InteractionComponent;
}

// Called when the game starts
void UInteractionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	bIsLocalPlayer = IsLocalPlayer();
	CreateInteractionWidget();
}

// Called every frame
void UInteractionManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsLocalPlayer)
		FindBestInteractableCandidate();
}

void UInteractionManagerComponent::RegisterInteractable(UInteractableComponent* Component)
{
	if (!IsValid(Component))
		return;

	if (!IsLocalPlayer())
		return;

	if (Component->InteractionTriggerInfo.InteractionTriggerType == EInteractionTriggerType::AutoInteract)
	{
		BestInteractableCandidate = nullptr;
		InteractWithInteractableComponent(Component);
		return;
	}

	if (!Interactables.Contains(Component))
	{
		Interactables.AddUnique(Component);
	}
}

void UInteractionManagerComponent::UnregisterInteractable(UInteractableComponent* Component)
{
	if (!IsValid(Component))
		return;

	if (!IsLocalPlayer())
		return;

	if (BestInteractableCandidate == Component)
	{
		DisableInteractionWidget();
		BestInteractableCandidate->SetInteractionEnable(false);
		BestInteractableCandidate = nullptr;
	}

	if (UsingInteractable == Component)
	{
		const EInteractionTriggerType TriggerType = Component->InteractionTriggerInfo.InteractionTriggerType;
		switch (TriggerType)
		{
		case EInteractionTriggerType::Hold:
			InteractionStop();
			break;

		case EInteractionTriggerType::PressMulti:
			ResetInteractionCounter();
			break;
		}
	}
	
	if (Interactables.Contains(Component))
	{
		Interactables.Remove(Component);
	}
}

bool UInteractionManagerComponent::IsLocalPlayer() const
{
	if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
	{
		if (OwningPawn->GetController() && OwningPawn->GetController()->IsLocalPlayerController())
		{
			return true;
		}
	}

	return false;
}

void UInteractionManagerComponent::CreateInteractionWidget()
{
	if (!IsLocalPlayer())
		return;

	InteractionWidgetComponent = NewObject<UInteractionWidgetComponent>(GetOwner());
	InteractionWidgetComponent->RegisterComponent();
	InteractionWidgetComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	InteractionWidgetComponent->SetVisibility(false);

	TSoftClassPtr<UInteractionUserWidget> TempWidgetClass = WidgetClass;
	TWeakObjectPtr<UInteractionWidgetComponent> WeakedComponent(InteractionWidgetComponent);
	
	UAssetManager::GetStreamableManager().RequestAsyncLoad(WidgetClass.ToSoftObjectPath(), [&, WeakedComponent, TempWidgetClass]()
		{
			WeakedComponent.Get()->SetWidgetClass(TempWidgetClass.Get());
			InteractionWidget = Cast<UInteractionUserWidget>(WeakedComponent.Get()->GetWidget());
		}
	, FStreamableManager::DefaultAsyncLoadPriority);
}

void UInteractionManagerComponent::InteractWithInteractableComponent(UInteractableComponent* Component)
{
	if (!ensure(IsValid(Component)))
		return;

	Component->SetInteractionEnable(false);
	DisableInteractionWidget();
	if (IsValid(InteractionWidget))
	{
		InteractionWidget->StopInteraction();
	}

	if (GetOwner()->HasAuthority())
	{
		Multi_InteractWithInteractableComponent(Component);
	}
	else
	{
		Server_InteractWithInteractableComponent(Component);
	}
}

void UInteractionManagerComponent::EnableInteractionWidget(UInteractableComponent* Component)
{
	if (!IsValid(Component))
		return;

	const EInteractionTriggerType TriggerType = Component->InteractionTriggerInfo.InteractionTriggerType;
	if (TriggerType == EInteractionTriggerType::AutoInteract)
		return;

	if (!Component->InteractionVisualPresentationInfo.UsesWidget())
		return;

	GetWorld()->GetTimerManager().ClearTimer(DetachWidgetTimerHandle);
	if (!IsValid(InteractionWidgetComponent))
		return;

	InteractionWidgetComponent->AttachToComponent(Component, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	InteractionWidgetComponent->SetRelativeTransform(Component->InteractionVisualPresentationInfo.WidgetOffset);
	InteractionWidgetComponent->SetVisibility(true);
}

void UInteractionManagerComponent::DisableInteractionWidget()
{
	if (!IsValid(InteractionWidgetComponent))
		return;

	InteractionWidgetComponent->SetVisibility(false);
	if (IsValid(InteractionWidget))
	{
		InteractionWidget->StopInteraction();
		GetWorld()->GetTimerManager().ClearTimer(DetachWidgetTimerHandle);
		DetachWidgetTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick([&]
			{
				InteractionWidgetComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			});
	}
}

void UInteractionManagerComponent::InteractionStart()
{
	if (!IsValid(BestInteractableCandidate))
		return;

	const EInteractionTriggerType TriggerType = BestInteractableCandidate->InteractionTriggerInfo.InteractionTriggerType;
	switch (TriggerType)
	{
		case EInteractionTriggerType::Press:
			InteractWithInteractableComponent(BestInteractableCandidate);
			break;

		case EInteractionTriggerType::Hold:
		{
			UsingInteractable = BestInteractableCandidate;
			const float HoldDuration = BestInteractableCandidate->InteractionTriggerInfo.HoldDuration;
			GetWorld()->GetTimerManager().ClearTimer(HoldInteractionTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(HoldInteractionTimerHandle, this, &UInteractionManagerComponent::ExecuteInteraction, HoldDuration);
			if (IsValid(InteractionWidget))
			{
				InteractionWidget->StartInteraction(HoldDuration);
			}
			break;
		}
	
		case EInteractionTriggerType::PressMulti:
		{
			UsingInteractable = BestInteractableCandidate;
			GetWorld()->GetTimerManager().ClearTimer(ResetInteractionCounterTimerHandle);
			FInteractionTriggerInfo& TriggerInfo = BestInteractableCandidate->InteractionTriggerInfo;
			const float ResetDelay = TriggerInfo.PressResetDelay;
			GetWorld()->GetTimerManager().SetTimer(ResetInteractionCounterTimerHandle, this, &UInteractionManagerComponent::ResetInteractionCounter, ResetDelay);
			TriggerInfo.CurrentPressedAmount++;
			if (IsValid(InteractionWidget))
			{
				InteractionWidget->UpdateInteractionCounter(TriggerInfo.CurrentPressedAmount, TriggerInfo.PressAmount);
			}

			if (TriggerInfo.CurrentPressedAmount == TriggerInfo.PressAmount)
			{
				ExecuteInteraction();
			}
			break;
		}
	}
}

void UInteractionManagerComponent::InteractionStop()
{
	if (!IsValid(UsingInteractable))
		return;

	if (UsingInteractable->InteractionTriggerInfo.InteractionTriggerType != EInteractionTriggerType::Hold)
		return;

	if (IsValid(InteractionWidget))
	{
		InteractionWidget->StopInteraction();
	}
	GetWorld()->GetTimerManager().ClearTimer(HoldInteractionTimerHandle);
	UsingInteractable = nullptr;
}

void UInteractionManagerComponent::ExecuteInteraction()
{
	if (IsValid(UsingInteractable))
	{
		UsingInteractable->InteractionTriggerInfo.CurrentPressedAmount = 0;
		InteractWithInteractableComponent(UsingInteractable);
		UsingInteractable = nullptr;
	}
}

void UInteractionManagerComponent::ResetInteractionCounter()
{
	if (!IsValid(UsingInteractable))
		return;

	UsingInteractable->InteractionTriggerInfo.CurrentPressedAmount = 0;
	UsingInteractable = nullptr;
	if (IsValid(InteractionWidget))
	{
		InteractionWidget->StopInteraction();
	}
}

void UInteractionManagerComponent::FindBestInteractableCandidate()
{
	if (IsValid(UsingInteractable))
		return;

	if (Interactables.Num() <= 0)
	{
		if (!IsValid(BestInteractableCandidate))
			return;

		BestInteractableCandidate->SetInteractionEnable(false);
		DisableInteractionWidget();
		BestInteractableCandidate = nullptr;
		OnInteractableUpdated.Broadcast(nullptr);
		return;
	}

	float BestDot = -1.0f;
	UInteractableComponent* NewBestCandidate = nullptr;
	for (UInteractableComponent* Component : Interactables)
	{
		const FVector& InteractableLocation = Component->GetComponentLocation();
		const FVector& InteractionSourceLocation = GetInteractionSourceLocation();
		const FVector& InteractionDirection = (InteractableLocation - InteractionSourceLocation).GetSafeNormal();
		const float SourceDirectionDot = FVector::DotProduct(InteractionDirection, GetLookAtDirection());
		const bool bSourceDirectionMatch = InteractionHalfAngle <= 0.0f ? true : SourceDirectionDot > FMath::Cos(FMath::DegreesToRadians(InteractionHalfAngle));
		const bool bIsBestDot = SourceDirectionDot > BestDot;
		bool bInteractionAngleMatch = Component->AcceptableInteractionHalfAngle == 0.0f;
		if (!bInteractionAngleMatch)
		{
			const FVector2D SourceDirectionToInteractable((GetComponentLocation() - InteractableLocation).GetSafeNormal());
			const FVector2D InteractableDirection((Component->GetForwardVector()).GetSafeNormal());
			const float InteractionAngleSourceDot = FVector2D::DotProduct(SourceDirectionToInteractable, InteractableDirection);
			bInteractionAngleMatch = InteractionAngleSourceDot >= FMath::Cos(FMath::DegreesToRadians(Component->AcceptableInteractionHalfAngle));
		}

		if (bSourceDirectionMatch && bIsBestDot && bInteractionAngleMatch)
		{
			BestDot = SourceDirectionDot;
			if (Component->bCheckLineOfSight)
			{
				if (!PerformTrace(GetLineOfSightSourceLocation(), InteractableLocation, Component->GetOwner()))
				{
					NewBestCandidate = Component;
				}
			}
			else
			{
				NewBestCandidate = Component;
			}
		}
	}

	if (IsValid(NewBestCandidate))
	{
		if (IsValid(BestInteractableCandidate))
		{
			if (NewBestCandidate != BestInteractableCandidate)
			{
				BestInteractableCandidate->SetInteractionEnable(false);
				BestInteractableCandidate = NewBestCandidate;
				BestInteractableCandidate->SetInteractionEnable(true);
				EnableInteractionWidget(BestInteractableCandidate);
				OnInteractableUpdated.Broadcast(BestInteractableCandidate);
			}
		}
		else
		{
			BestInteractableCandidate = NewBestCandidate;
			BestInteractableCandidate->SetInteractionEnable(true);
			EnableInteractionWidget(BestInteractableCandidate);
			OnInteractableUpdated.Broadcast(BestInteractableCandidate);
		}
	}
	else
	{
		if (IsValid(BestInteractableCandidate))
		{
			BestInteractableCandidate->SetInteractionEnable(false);
			BestInteractableCandidate = nullptr;
			OnInteractableUpdated.Broadcast(nullptr);
			DisableInteractionWidget();
		}
	}
}

FVector UInteractionManagerComponent::GetInteractionSourceLocation()
{
	const EInteractionLocationSource LocationSource = InteractionSourceLocationInfo.LocationType;
	switch (LocationSource)
	{
	case EInteractionLocationSource::InteractionComponent:
		return GetComponentLocation();

	case EInteractionLocationSource::OwningActor:
		return GetOwner()->GetActorLocation();

	case EInteractionLocationSource::OtherComponent:
		if (!ensure(InteractionSourceLocationInfo.ComponentTag.IsNone()))
		{
			TArray<UActorComponent*> Components = GetOwner()->GetComponentsByTag(USceneComponent::StaticClass(), InteractionSourceLocationInfo.ComponentTag);
			if (ensure(Components.IsValidIndex(0)))
			{
				return Cast<USceneComponent>(Components[0])->GetComponentLocation();
			}
		}

	case EInteractionLocationSource::Camera:
		return UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
	}

	return GetComponentLocation();
}

FVector UInteractionManagerComponent::GetLineOfSightSourceLocation()
{
	const EInteractionLocationSource LocationSource = LineOfSightSourceLocationInfo.LocationType;
	switch (LocationSource)
	{
	case EInteractionLocationSource::InteractionComponent:
		return GetComponentLocation();

	case EInteractionLocationSource::OwningActor:
		return GetOwner()->GetActorLocation();

	case EInteractionLocationSource::OtherComponent:
		if (!ensure(LineOfSightSourceLocationInfo.ComponentTag.IsNone()))
		{
			TArray<UActorComponent*> Components = GetOwner()->GetComponentsByTag(USceneComponent::StaticClass(), LineOfSightSourceLocationInfo.ComponentTag);
			if (ensure(Components.IsValidIndex(0)))
			{
				return Cast<USceneComponent>(Components[0])->GetComponentLocation();
			}
		}

	case EInteractionLocationSource::Camera:
		return UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
	}

	return GetComponentLocation();
}

FVector UInteractionManagerComponent::GetLookAtDirection()
{
	EInteractionDirectionSource DirectionType = InteractionSourceDirectionInfo.DirectionType;
	switch (DirectionType)
	{
		case EInteractionDirectionSource::InteractionComponent:
			return GetForwardVector();

		case EInteractionDirectionSource::ControlRotation:
		{
			APawn* OwningPawn = Cast<APawn>(GetOwner());
			if (ensure(IsValid(OwningPawn)))
			{
				return OwningPawn->GetController()->GetControlRotation().Vector();
			}
		}

		case EInteractionDirectionSource::OtherComponent:
		{
			if (!ensure(InteractionSourceDirectionInfo.ComponentTag.IsNone()))
			{
				TArray<UActorComponent*> Components = GetOwner()->GetComponentsByTag(USceneComponent::StaticClass(), InteractionSourceDirectionInfo.ComponentTag);
				if (ensure(Components.IsValidIndex(0)))
				{
					return Cast<USceneComponent>(Components[0])->GetForwardVector();
				}
			}
		}

		case EInteractionDirectionSource::OwningActor:
			return GetOwner()->GetActorForwardVector();

		case EInteractionDirectionSource::Camera:
			return UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraRotation().Vector();
	}
	
	return GetForwardVector();
}

bool UInteractionManagerComponent::PerformTrace(const FVector& Start, const FVector& End, AActor* Against)
{
	FHitResult OutHit;
	FCollisionQueryParams Params(FName("InteractionTrace"));
	Params.bTraceComplex = LineOfSightTraceInfo.bTraceComplex;
	Params.bReturnPhysicalMaterial = true;
	Params.bReturnFaceIndex = !UPhysicsSettings::Get()->bSuppressFaceRemapTable;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(Against);
	
	FCollisionShape CollisionShape;
	FQuat TraceQuat = FQuat::Identity;
	EInteractionTraceShapeType TraceShape = LineOfSightTraceInfo.TraceShape;
	switch (TraceShape)
	{
	case EInteractionTraceShapeType::Line:
		break;
	case EInteractionTraceShapeType::Sphere:
		CollisionShape = FCollisionShape::MakeSphere(LineOfSightTraceInfo.Radius);
		break;

	case EInteractionTraceShapeType::Box:
		CollisionShape = FCollisionShape::MakeBox(LineOfSightTraceInfo.HalfSize);
		TraceQuat = GetLookAtDirection().ToOrientationQuat();
		break;

	case EInteractionTraceShapeType::Capsule:
		CollisionShape = FCollisionShape::MakeCapsule(LineOfSightTraceInfo.Radius, LineOfSightTraceInfo.HalfHeight);
		break;
	}
	
	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
	EInteractionTraceType TraceType = LineOfSightTraceInfo.TraceType;
	switch (TraceType)
	{
		case EInteractionTraceType::Channel:
		{
			ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(LineOfSightTraceInfo.TraceChannel);
			return World->SweepSingleByChannel(OutHit, Start, End, TraceQuat, CollisionChannel, CollisionShape, Params);
		}

		case EInteractionTraceType::Objects:
		{
			const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes = LineOfSightTraceInfo.ObjectTypes;
			TArray<TEnumAsByte<ECollisionChannel>> CollisionObjectTraces;
			CollisionObjectTraces.AddUninitialized(ObjectTypes.Num());

			for (auto Iter = ObjectTypes.CreateConstIterator(); Iter; ++Iter)
			{
				CollisionObjectTraces[Iter.GetIndex()] = UEngineTypes::ConvertToCollisionChannel(*Iter);
			}

			FCollisionObjectQueryParams ObjectParams;
			for (auto Iter = CollisionObjectTraces.CreateConstIterator(); Iter; ++Iter)
			{
				const ECollisionChannel& Channel = (*Iter);
				if (FCollisionObjectQueryParams::IsValidObjectQuery(Channel))
				{
					ObjectParams.AddObjectTypesToQuery(Channel);
				}
			}

			return World->SweepSingleByObjectType(OutHit, Start, End, TraceQuat, ObjectParams, CollisionShape, Params);
		}

		case EInteractionTraceType::Profile:
		{
			return World->SweepSingleByProfile(OutHit, Start, End, TraceQuat, LineOfSightTraceInfo.ProfileName, CollisionShape, Params);
		}
	}

	return false;
}

void UInteractionManagerComponent::Server_InteractWithInteractableComponent_Implementation(UInteractableComponent* Component)
{
	Multi_InteractWithInteractableComponent(Component);
}

bool UInteractionManagerComponent::Server_InteractWithInteractableComponent_Validate(UInteractableComponent* Component)
{
	if (!Component->IsActive())
		return false;

	if (!Component->IsOverlappingActor(GetOwner()))
		return false;
	
	const FVector& InteractableLocation = Component->GetComponentLocation();
	bool bInteractionAngleMatch = Component->AcceptableInteractionHalfAngle == 0.0f;
	if (!bInteractionAngleMatch)
	{
		const FVector2D SourceDirectionToInteractable((GetComponentLocation() - InteractableLocation).GetSafeNormal());
		const FVector2D InteractableDirection((Component->GetForwardVector()).GetSafeNormal());
		const float InteractionAngleSourceDot = FVector2D::DotProduct(SourceDirectionToInteractable, InteractableDirection);
		bInteractionAngleMatch = InteractionAngleSourceDot >= FMath::Cos(FMath::DegreesToRadians(Component->AcceptableInteractionHalfAngle));
	}

	if (!bInteractionAngleMatch)
		return false;

	if (Component->bCheckLineOfSight)
	{
		if (PerformTrace(GetLineOfSightSourceLocation(), InteractableLocation, Component->GetOwner()))
		{
			return false;
		}
	}

	return true;
}

void UInteractionManagerComponent::Multi_InteractWithInteractableComponent_Implementation(UInteractableComponent* Component)
{
	PreInteractedWithInteractable.Broadcast(Component, Component->GetOwner(), GetOwner());
	Component->Interact(GetOwner());
	PostInteractedWithInteractable.Broadcast(Component, Component->GetOwner(), GetOwner());
}