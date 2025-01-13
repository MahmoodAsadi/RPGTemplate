// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractableHelper.generated.h"

class UInteractableComponent;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableUpdated, UInteractableComponent*, InteractableComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractedWithInteractable, UInteractableComponent*, InteractableComponent, AActor*, OwningActor, UObject*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionActivate, UInteractableComponent*, InteractableComponent, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableInteracted, UInteractableComponent*, InteractableComponent, UObject*, Instigator);

UENUM(BlueprintType)
enum class EInteractionTriggerType : uint8
{
	// By Pressing Interaction Key.
	Press,

	// By Holding Interaction Key for certain Duration.
	Hold,

	// By Pressing Interaction Key Multiple Times.
	PressMulti,

	// No input needed, Auto Interact if Interaction condition met (Distance, Collision, Trace)
	AutoInteract
};

USTRUCT(BlueprintType)
struct FInteractionTriggerInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EInteractionTriggerType InteractionTriggerType = EInteractionTriggerType::Press;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "InteractionTriggerType == EInteractionTriggerType::Hold", EditConditionHides, Units = "s"))
	float HoldDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "InteractionTriggerType == EInteractionTriggerType::PressMulti", EditConditionHides))
	int PressAmount = 5;

	UPROPERTY(BlueprintReadWrite)
	int CurrentPressedAmount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "InteractionTriggerType == EInteractionTriggerType::PressMulti", EditConditionHides, Units = "s"))
	float PressResetDelay = 0.4f;

};

//EInteractionVisualPresentationType
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EInteractionVisualPresentationType : uint8
{
	NONE = 0 UMETA(Hidden),

	// Shows widget when Interaction available
	Widget = 1 << 0,

	// Outline Owning Actor when Interaction available
	Outline = 1 << 1
};
ENUM_CLASS_FLAGS(EInteractionVisualPresentationType);

USTRUCT(BlueprintType)
struct FInteractionVisualInfo
{
	GENERATED_BODY()

public:

	// Interaction availability visual presentation type
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "/Script/MultiPurposeInteractionSystem.EInteractionVisualPresentationType"))
	uint8 InteractionVisualPresentation = 0;

	// Widget Offset from Interaction component
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "InteractionVisualPresentation == 1 || InteractionVisualPresentation == 3", EditConditionHides))
	FTransform WidgetOffset;

	// Component tag used for owning actor components to create outline with
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "InteractionVisualPresentation == 2 || InteractionVisualPresentation == 3", EditConditionHides))
	FName ComponentTag = "InteractableOutline";

	bool UsesWidget() const
	{
		return InteractionVisualPresentation & uint8(EInteractionVisualPresentationType::Widget);
	}

	bool UsesOutline() const
	{
		return InteractionVisualPresentation & uint8(EInteractionVisualPresentationType::Outline);
	}
};


UENUM(BlueprintType)
enum class EInteractionLocationSource : uint8
{
	// Uses Interaction Component as source location.
	InteractionComponent,

	// Uses owning actor location as source location.
	OwningActor,

	/*
	* Uses Other Component as Source Location. (Find component by Tag)
	* Note: Will use Interaction Component location if couldn't find other Component by given Tag.
	*/
	OtherComponent,

	// Uses Camera (Camera Manager) location as source location.
	Camera
};


USTRUCT(BlueprintType)
struct FInteractionSourceLocationInfo
{
	GENERATED_BODY()

public:

	// Represent the source cocation of Interaction while calculating direction to Interactable object
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EInteractionLocationSource LocationType = EInteractionLocationSource::Camera;

	// Component Tag used on other component which need to be used as Interaction source location
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "LocationType == EInteractionLocationSource::OtherComponent", EditConditionHides))
	FName ComponentTag = "InteractionLocationSource";

};


UENUM(BlueprintType)
enum class EInteractionDirectionSource : uint8
{
	// Uses Interaction Component Direction.
	InteractionComponent,

	/*
	* Uses owner Control Rotation as Source Direction.
	* Note: Owner of the component should inherit from Pawn class, otherwise it won't work.
	*/
	ControlRotation,

	/*
	* Uses Other Component as Direction. (Find component by Tag)
	* Useful if want to use Camera or Spring Arm Direction.
	* Note: Will use Interaction Component Direction if couldn't find other Component by given Tag.
	*/
	OtherComponent,

	// Uses owning actor direction as source location.
	OwningActor,

	// Uses Camera (Camera Manager) Direction.
	Camera
};


USTRUCT(BlueprintType)
struct FInteractionSourceDirectionInfo
{
	GENERATED_BODY()

public:

	// Represent the Interaction Direction source, Need to comparing this vector with direction to the Interactable object
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EInteractionDirectionSource DirectionType = EInteractionDirectionSource::ControlRotation;

	// Component Tag used on other component which need to be used as Interaction direction source
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "DirectionType == EInteractionDirectionSource::OtherComponent", EditConditionHides))
	FName ComponentTag = "InteractionDirectionSource";

};


UENUM(BlueprintType)
enum class EInteractionTraceType : uint8
{
	// Trace by given channel type
	Channel,

	// Trace by given object types
	Objects,

	// Trace by profile
	Profile
};


UENUM(BlueprintType)
enum class EInteractionTraceShapeType : uint8
{
	// Line Trace
	Line,

	// Sphere Trace
	Sphere,

	// Box Trace
	Box,

	// Capsule
	Capsule
};


USTRUCT(BlueprintType)
struct FInteractionTraceInfo
{
	GENERATED_BODY()

public:

	// Type of the trace used to check line of sight between Interaction source and Interactable object
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EInteractionTraceType TraceType = EInteractionTraceType::Channel;

	// Trace channel used to perform the trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "TraceType == EInteractionTraceType::Channel", EditConditionHides))
	TEnumAsByte<ETraceTypeQuery> TraceChannel = TraceTypeQuery1;

	// Object types used to perfrom trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "TraceType == EInteractionTraceType::Objects", EditConditionHides))
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	// Profile name used to perform trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "TraceType == EInteractionTraceType::Profile", EditConditionHides))
	FName ProfileName = FName("BlockAllDynamics");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bTraceComplex = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EInteractionTraceShapeType TraceShape = EInteractionTraceShapeType::Line;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "TraceShape == EInteractionTraceShapeType::Sphere || TraceShape == EInteractionTraceShapeType::Capsule", EditConditionHides))
	float Radius = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "TraceShape == EInteractionTraceShapeType::Box", EditConditionHides))
	FVector HalfSize = FVector(4.0f, 4.0f, 4.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "TraceShape == EInteractionTraceShapeType::Capsule", EditConditionHides))
	float HalfHeight = 12.0f;
};



/**
 * 
 */
UCLASS()
class MULTIPURPOSEINTERACTIONSYSTEM_API UInteractableHelper : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "Interaction|Helper")
	static bool IsWidgetType(FInteractionVisualInfo InValue)
	{
		return InValue.UsesWidget();
	}

	UFUNCTION(BlueprintPure, Category = "Interaction|Helper")
	static bool IsOutlineType(FInteractionVisualInfo InValue)
	{
		return InValue.UsesOutline();
	}
};