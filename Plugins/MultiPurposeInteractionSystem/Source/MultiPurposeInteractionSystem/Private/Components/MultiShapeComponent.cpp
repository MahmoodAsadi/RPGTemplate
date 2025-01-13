// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/MultiShapeComponent.h"
#include "CollisionShape.h"
#include "PhysicsEngine/BodySetup.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "PhysicsEngine/SphereElem.h"
#include "SceneManagement.h"
#include "PrimitiveSceneProxy.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(MultiShapeComponent)

UMultiShapeComponent::UMultiShapeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SphereRadius = 32.0f;

	ShapeColor = FColor(255, 0, 0, 255);
	BoxExtent = FVector(32.0f, 32.0f, 32.0f);

	CapsuleRadius = 22.0f;
	CapsuleHalfHeight = 44.0f;

	bUseEditorCompositing = true;
}

template <EShapeBodySetupHelper UpdateBodySetupAction, typename BodySetupType>
bool InvalidateOrUpdateSphereBodySetup(BodySetupType& ShapeBodySetup, bool bUseArchetypeBodySetup, float SphereRadius)
{
	check((bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::InvalidateSharingIfStale) || (!bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup));
	check(ShapeBodySetup->AggGeom.SphereElems.Num() == 1);
	FKSphereElem* SphereElem = ShapeBodySetup->AggGeom.SphereElems.GetData();

	// check for mal formed values
	float Radius = SphereRadius;
	if (Radius < UE_KINDA_SMALL_NUMBER)
	{
		Radius = 0.1f;
	}

	if (UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup)
	{
		// now set the PhysX data values
		SphereElem->Center = FVector::ZeroVector;
		SphereElem->Radius = Radius;
	}
	else
	{
		if (SphereElem->Radius != Radius)
		{
			ShapeBodySetup = nullptr;
			bUseArchetypeBodySetup = false;
		}
	}

	return bUseArchetypeBodySetup;
}

template <EShapeBodySetupHelper UpdateBodySetupAction, typename BodySetupType>
bool InvalidateOrUpdateBoxBodySetup(BodySetupType& ShapeBodySetup, bool bUseArchetypeBodySetup, FVector BoxExtent)
{
	check((bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::InvalidateSharingIfStale) || (!bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup));
	check(ShapeBodySetup->AggGeom.BoxElems.Num() == 1);
	FKBoxElem* se = ShapeBodySetup->AggGeom.BoxElems.GetData();

	// @todo do we allow this now?
	// check for malformed values
	if (BoxExtent.X < UE_KINDA_SMALL_NUMBER)
	{
		BoxExtent.X = 1.0f;
	}

	if (BoxExtent.Y < UE_KINDA_SMALL_NUMBER)
	{
		BoxExtent.Y = 1.0f;
	}

	if (BoxExtent.Z < UE_KINDA_SMALL_NUMBER)
	{
		BoxExtent.Z = 1.0f;
	}

	float XExtent = BoxExtent.X * 2.f;
	float YExtent = BoxExtent.Y * 2.f;
	float ZExtent = BoxExtent.Z * 2.f;

	if (UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup)
	{
		// now set the PhysX data values
		se->SetTransform(FTransform::Identity);
		se->X = XExtent;
		se->Y = YExtent;
		se->Z = ZExtent;
	}
	else if (se->X != XExtent || se->Y != YExtent || se->Z != ZExtent)
	{
		ShapeBodySetup = nullptr;
		bUseArchetypeBodySetup = false;
	}

	return bUseArchetypeBodySetup;
}

template <EShapeBodySetupHelper UpdateBodySetupAction, typename BodySetupType>
bool InvalidateOrUpdateCapsuleBodySetup(BodySetupType& ShapeBodySetup, bool bUseArchetypeBodySetup, float CapsuleRadius, float CapsuleHalfHeight)
{
	check((bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::InvalidateSharingIfStale) || (!bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup));
	check(ShapeBodySetup->AggGeom.SphylElems.Num() == 1);
	FKSphylElem* SE = ShapeBodySetup->AggGeom.SphylElems.GetData();

	//SphylElem uses height from center of capsule spheres, but UCapsuleComponent uses halfHeight from end of the sphere
	const float Length = 2 * FMath::Max(CapsuleHalfHeight - CapsuleRadius, 0.f);

	if (UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup)
	{
		SE->SetTransform(FTransform::Identity);
		SE->Radius = CapsuleRadius;
		SE->Length = Length;
	}
	else
	{
		if (SE->Radius != CapsuleRadius || SE->Length != Length)
		{
			ShapeBodySetup = nullptr;
			bUseArchetypeBodySetup = false;
		}
	}

	return bUseArchetypeBodySetup;
}

void UMultiShapeComponent::SetSphereRadius(float InSphereRadius, bool bUpdateOverlaps)
{
	if (CollisionShape != EBodyShapeType::Sphere)
		return;

	SphereRadius = InSphereRadius;
	UpdateBounds();
	UpdateBodySetup();
	MarkRenderStateDirty();

	if (bPhysicsStateCreated)
	{
		// Update physics engine collision shapes
		BodyInstance.UpdateBodyScale(GetComponentTransform().GetScale3D(), true);

		if (bUpdateOverlaps && IsCollisionEnabled() && GetOwner())
		{
			UpdateOverlaps();
		}
	}
}

float UMultiShapeComponent::GetScaledSphereRadius() const
{
	if (CollisionShape != EBodyShapeType::Sphere)
		return 0.0f;

	return SphereRadius * GetShapeScale();
}

float UMultiShapeComponent::GetUnscaledSphereRadius() const
{
	if (CollisionShape != EBodyShapeType::Sphere)
		return 0.0f;

	return SphereRadius;
}

float UMultiShapeComponent::GetShapeScale() const
{
	return GetComponentTransform().GetMinimumAxisScale();
}

void UMultiShapeComponent::SetBoxExtent(FVector InBoxExtent, bool bUpdateOverlaps)
{
	if (CollisionShape != EBodyShapeType::Box)
		return;

	BoxExtent = InBoxExtent;
	UpdateBounds();
	MarkRenderStateDirty();
	UpdateBodySetup();

	// do this if already created
	// otherwise, it hasn't been really created yet
	if (bPhysicsStateCreated)
	{
		// Update physics engine collision shapes
		BodyInstance.UpdateBodyScale(GetComponentTransform().GetScale3D(), true);

		if (bUpdateOverlaps && IsCollisionEnabled() && GetOwner())
		{
			UpdateOverlaps();
		}
	}
}

void UMultiShapeComponent::SetLineThickness(float Thickness)
{
	if (CollisionShape != EBodyShapeType::Box)
		return;

	LineThickness = Thickness;
	MarkRenderStateDirty();
}

FVector UMultiShapeComponent::GetScaledBoxExtent() const
{
	if (CollisionShape != EBodyShapeType::Box)
		return FVector::ZeroVector;

	return BoxExtent * GetComponentTransform().GetScale3D();
}

FVector UMultiShapeComponent::GetUnscaledBoxExtent() const
{
	if (CollisionShape != EBodyShapeType::Box)
		return FVector::ZeroVector;

	return BoxExtent;
}

void UMultiShapeComponent::SetCapsuleSize(float InRadius, float InHalfHeight, bool bUpdateOverlaps)
{
	CapsuleHalfHeight = FMath::Max3(0.f, InHalfHeight, InRadius);
	CapsuleRadius = FMath::Max(0.f, InRadius);
	UpdateBounds();
	UpdateBodySetup();
	MarkRenderStateDirty();

	// do this if already created
	// otherwise, it hasn't been really created yet
	if (bPhysicsStateCreated)
	{
		// Update physics engine collision shapes
		BodyInstance.UpdateBodyScale(GetComponentTransform().GetScale3D(), true);

		if (bUpdateOverlaps && IsCollisionEnabled() && GetOwner())
		{
			UpdateOverlaps();
		}
	}
}

void UMultiShapeComponent::SetCapsuleRadius(float Radius, bool bUpdateOverlaps)
{
	SetCapsuleSize(Radius, GetUnscaledCapsuleHalfHeight(), bUpdateOverlaps);
}

void UMultiShapeComponent::SetCapsuleHalfHeight(float HalfHeight, bool bUpdateOverlaps)
{
	SetCapsuleSize(GetUnscaledCapsuleRadius(), HalfHeight, bUpdateOverlaps);
}

float UMultiShapeComponent::GetScaledCapsuleRadius() const
{
	const FVector& ComponentScale = GetComponentTransform().GetScale3D();
	return CapsuleRadius * UE_REAL_TO_FLOAT(ComponentScale.X < ComponentScale.Y ? ComponentScale.X : ComponentScale.Y);
}

float UMultiShapeComponent::GetScaledCapsuleHalfHeight() const
{
	return CapsuleHalfHeight * UE_REAL_TO_FLOAT(GetComponentTransform().GetScale3D().Z);
}

float UMultiShapeComponent::GetScaledCapsuleHalfHeight_WithoutHemisphere() const
{
	return GetScaledCapsuleHalfHeight() - GetScaledCapsuleRadius();
}

void UMultiShapeComponent::GetScaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const
{
	OutRadius = GetScaledCapsuleRadius();
	OutHalfHeight = GetScaledCapsuleHalfHeight();
}

void UMultiShapeComponent::GetScaledCapsuleSize_WithoutHemisphere(float& OutRadius, float& OutHalfHeightWithoutHemisphere) const
{
	OutRadius = GetScaledCapsuleRadius();
	OutHalfHeightWithoutHemisphere = GetScaledCapsuleHalfHeight_WithoutHemisphere();
}

void UMultiShapeComponent::GetUnscaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const
{
	OutRadius = CapsuleRadius;
	OutHalfHeight = CapsuleHalfHeight;
}

void UMultiShapeComponent::GetUnscaledCapsuleSize_WithoutHemisphere(float& OutRadius, float& OutHalfHeightWithoutHemisphere) const
{
	OutRadius = CapsuleRadius;
	OutHalfHeightWithoutHemisphere = CapsuleHalfHeight - CapsuleRadius;
}

bool UMultiShapeComponent::IsZeroExtent() const
{
	switch (CollisionShape)
	{
	case EBodyShapeType::Sphere:
		return SphereRadius == 0.f;

	case EBodyShapeType::Box:
		return BoxExtent.IsZero();

	case EBodyShapeType::Capsule:
		return (CapsuleRadius == 0.f) && (CapsuleHalfHeight == 0.f);
	}

	return false;
}

FPrimitiveSceneProxy* UMultiShapeComponent::CreateSceneProxy()
{
	/** Represents a sphere to the scene manager. */
	class FSphereSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		/** Initialization constructor. */
		FSphereSceneProxy(const UMultiShapeComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, SphereColor(InComponent->ShapeColor)
			, SphereRadius(InComponent->SphereRadius)
		{
			bWillEverBeLit = false;
		}

		// FPrimitiveSceneProxy interface.

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_SphereSceneProxy_GetDynamicMeshElements);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

					const FMatrix& LocalToWorld = GetLocalToWorld();
					const FLinearColor DrawSphereColor = GetViewSelectionColor(SphereColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					// Taking into account the min and maximum drawing distance
					const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorld.GetOrigin()).SizeSquared();
					if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
					{
						continue;
					}

					float AbsScaleX = LocalToWorld.GetScaledAxis(EAxis::X).Size();
					float AbsScaleY = LocalToWorld.GetScaledAxis(EAxis::Y).Size();
					float AbsScaleZ = LocalToWorld.GetScaledAxis(EAxis::Z).Size();
					float MinAbsScale = FMath::Min3(AbsScaleX, AbsScaleY, AbsScaleZ);

					FVector ScaledX = LocalToWorld.GetUnitAxis(EAxis::X) * MinAbsScale;
					FVector ScaledY = LocalToWorld.GetUnitAxis(EAxis::Y) * MinAbsScale;
					FVector ScaledZ = LocalToWorld.GetUnitAxis(EAxis::Z) * MinAbsScale;

					const int32 SphereSides = FMath::Clamp<int32>(SphereRadius / 4.f, 16, 64);
					DrawCircle(PDI, LocalToWorld.GetOrigin(), ScaledX, ScaledY, DrawSphereColor, SphereRadius, SphereSides, SDPG_World);
					DrawCircle(PDI, LocalToWorld.GetOrigin(), ScaledX, ScaledZ, DrawSphereColor, SphereRadius, SphereSides, SDPG_World);
					DrawCircle(PDI, LocalToWorld.GetOrigin(), ScaledY, ScaledZ, DrawSphereColor, SphereRadius, SphereSides, SDPG_World);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bVisibleForSelection = !bDrawOnlyIfSelected || IsSelected();
			const bool bVisibleForShowFlags = true; // @TODO

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bVisibleForSelection && bVisibleForShowFlags) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const uint32				bDrawOnlyIfSelected : 1;
		const FColor				SphereColor;
		const float					SphereRadius;
	};

	/** Represents a box to the scene manager. */
	class FBoxSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FBoxSceneProxy(const UMultiShapeComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, BoxExtents(InComponent->BoxExtent)
			, BoxColor(InComponent->ShapeColor)
			, LineThickness(InComponent->LineThickness)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_BoxSceneProxy_GetDynamicMeshElements);

			const FMatrix& LocalToWorld = GetLocalToWorld();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					const FLinearColor DrawColor = GetViewSelectionColor(BoxColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), BoxExtents, DrawColor, SDPG_World, LineThickness);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const uint32	bDrawOnlyIfSelected : 1;
		const FVector	BoxExtents;
		const FColor	BoxColor;
		const float LineThickness;
	};

	/** Represents a UCapsuleComponent to the scene manager. */
	class FDrawCylinderSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FDrawCylinderSceneProxy(const UMultiShapeComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, CapsuleRadius(InComponent->GetScaledCapsuleRadius())
			, CapsuleHalfHeight(InComponent->GetScaledCapsuleHalfHeight())
			, ShapeColor(InComponent->ShapeColor)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_GetDynamicMeshElements_DrawDynamicElements);


			const FMatrix& LocalToWorld = GetLocalToWorld();
			const int32 CapsuleSides = FMath::Clamp<int32>(CapsuleRadius / 4.f, 16, 64);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{

				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					const FLinearColor DrawCapsuleColor = GetViewSelectionColor(ShapeColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetUnitAxis(EAxis::X), LocalToWorld.GetUnitAxis(EAxis::Y), LocalToWorld.GetUnitAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const uint32	bDrawOnlyIfSelected : 1;
		const float		CapsuleRadius;
		const float		CapsuleHalfHeight;
		const FColor	ShapeColor;
	};

	switch (CollisionShape)
	{
	case EBodyShapeType::Sphere:
		return new FSphereSceneProxy(this);

	case EBodyShapeType::Box:
		return new FBoxSceneProxy(this);

	case EBodyShapeType::Capsule:
		return new FDrawCylinderSceneProxy(this);
	}

	return new FSphereSceneProxy(this);
}

FCollisionShape UMultiShapeComponent::GetCollisionShape(float Inflation) const
{
	switch (CollisionShape)
	{
		case EBodyShapeType::Sphere:
		{
			const float Radius = FMath::Max(0.f, GetScaledSphereRadius() + Inflation);
			return FCollisionShape::MakeSphere(Radius);
		}

		case EBodyShapeType::Box:
		{
			FVector Extent = GetScaledBoxExtent() + Inflation;
			if (Inflation < 0.f)
			{
				// Don't shrink below zero size.
				Extent = Extent.ComponentMax(FVector::ZeroVector);
			}

			return FCollisionShape::MakeBox(Extent);
		}

		case EBodyShapeType::Capsule:
		{
			const float Radius = FMath::Max(0.f, GetScaledCapsuleRadius() + Inflation);
			const float HalfHeight = FMath::Max(0.f, GetScaledCapsuleHalfHeight() + Inflation);
			return FCollisionShape::MakeCapsule(Radius, HalfHeight);
		}
	}

	return FCollisionShape::MakeSphere(0.f);
}

bool UMultiShapeComponent::AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const
{
	switch (CollisionShape)
	{
		case EBodyShapeType::Sphere:
		{
			// All rotations are equal when scale is uniform.
			// Not detecting rotations around non-uniform scale.
			return Scale3D.GetAbs().AllComponentsEqual() || A.Equals(B);
		}

		case EBodyShapeType::Capsule:
		{
			if (Scale3D.X != Scale3D.Y)
			{
				return false;
			}

			const FVector AUp = A.GetAxisZ();
			const FVector BUp = B.GetAxisZ();
			return AUp.Equals(BUp);
		}
	}

	return false;
}

FBoxSphereBounds UMultiShapeComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	switch (CollisionShape)
	{
	case EBodyShapeType::Sphere:
		return FBoxSphereBounds(FVector::ZeroVector, FVector(SphereRadius), SphereRadius).TransformBy(LocalToWorld);

	case EBodyShapeType::Box:
		return FBoxSphereBounds(FBox(-BoxExtent, BoxExtent)).TransformBy(LocalToWorld);

	case EBodyShapeType::Capsule:
		return FBoxSphereBounds(FVector::ZeroVector, FVector(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight), CapsuleHalfHeight).TransformBy(LocalToWorld);
	}

	return FBoxSphereBounds(FVector::ZeroVector, FVector(SphereRadius), SphereRadius).TransformBy(LocalToWorld);
}

void UMultiShapeComponent::CalcBoundingCylinder(float& CylinderRadius, float& CylinderHalfHeight) const
{
	switch (CollisionShape)
	{
		case EBodyShapeType::Sphere:
		{
			CylinderRadius = SphereRadius * GetComponentTransform().GetMaximumAxisScale();
			CylinderHalfHeight = CylinderRadius;
		}
		break;

		case EBodyShapeType::Box:
		break;
		
		case EBodyShapeType::Capsule:
		{
			const FVector& ComponentScale = GetComponentTransform().GetScale3D();
			const float CapsuleEndCapCenter = FMath::Max(CapsuleHalfHeight - CapsuleRadius, 0.f);
			const FVector ZAxis = GetComponentTransform().TransformVectorNoScale(FVector(0.f, 0.f, CapsuleEndCapCenter * ComponentScale.Z));

			const float ScaledRadius = CapsuleRadius * (ComponentScale.X > ComponentScale.Y ? ComponentScale.X : ComponentScale.Y);

			CylinderRadius = ScaledRadius + FMath::Sqrt(FMath::Square(ZAxis.X) + FMath::Square(ZAxis.Y));
			CylinderHalfHeight = ScaledRadius + ZAxis.Z;
		}
		break;
	}
}

void UMultiShapeComponent::UpdateBodySetup()
{
	ShapeBodySetup = nullptr;

	switch (CollisionShape)
	{
		case EBodyShapeType::Sphere:

			CreateShapeBodySetupIfNeeded<FKSphereElem>();

			if (PrepareSharedBodySetup<UMultiShapeComponent>())
			{
				bUseArchetypeBodySetup = InvalidateOrUpdateSphereBodySetup<EShapeBodySetupHelper::InvalidateSharingIfStale>(ShapeBodySetup, bUseArchetypeBodySetup, SphereRadius);
			}

			CreateShapeBodySetupIfNeeded<FKSphereElem>();

			if (!bUseArchetypeBodySetup)
			{
				InvalidateOrUpdateSphereBodySetup<EShapeBodySetupHelper::UpdateBodySetup>(ShapeBodySetup, bUseArchetypeBodySetup, SphereRadius);
			}

		break;

		case EBodyShapeType::Box:

			CreateShapeBodySetupIfNeeded<FKBoxElem>();

			if (PrepareSharedBodySetup<UMultiShapeComponent>())
			{
				bUseArchetypeBodySetup = InvalidateOrUpdateBoxBodySetup<EShapeBodySetupHelper::InvalidateSharingIfStale>(ShapeBodySetup, bUseArchetypeBodySetup, BoxExtent);
			}
			
			CreateShapeBodySetupIfNeeded<FKBoxElem>();

			if (!bUseArchetypeBodySetup)
			{
				InvalidateOrUpdateBoxBodySetup<EShapeBodySetupHelper::UpdateBodySetup>(ShapeBodySetup, bUseArchetypeBodySetup, BoxExtent);
			}

		break;

	case EBodyShapeType::Capsule:

		CreateShapeBodySetupIfNeeded<FKSphylElem>();

		if (PrepareSharedBodySetup<UMultiShapeComponent>())
		{
			bUseArchetypeBodySetup = InvalidateOrUpdateCapsuleBodySetup<EShapeBodySetupHelper::InvalidateSharingIfStale>(ShapeBodySetup, bUseArchetypeBodySetup, CapsuleRadius, CapsuleHalfHeight);
		}

		CreateShapeBodySetupIfNeeded<FKSphylElem>();

		if (!bUseArchetypeBodySetup)
		{
			InvalidateOrUpdateCapsuleBodySetup<EShapeBodySetupHelper::UpdateBodySetup>(ShapeBodySetup, bUseArchetypeBodySetup, CapsuleRadius, CapsuleHalfHeight);
		}

		break;
	}
}

void UMultiShapeComponent::PostLoad()
{
	Super::PostLoad();

	// Ensure this value is clamped only in the case where we're not re-running construction scripts.
	if (!GIsReconstructingBlueprintInstances)
	{
		CapsuleHalfHeight = FMath::Max3(0.f, CapsuleHalfHeight, CapsuleRadius);
	}
}

#if WITH_EDITOR
void UMultiShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// We only want to modify the property that was changed at this point
	// things like propagation from CDO to instances don't work correctly if changing one property causes a different property to change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMultiShapeComponent, CapsuleHalfHeight))
	{
		CapsuleHalfHeight = FMath::Max3(0.f, CapsuleHalfHeight, CapsuleRadius);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMultiShapeComponent, CapsuleRadius))
	{
		CapsuleRadius = FMath::Clamp(CapsuleRadius, 0.f, CapsuleHalfHeight);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UMultiShapeComponent::PreEditChange(FProperty* PropertyThatWillChange)
{
	if (PropertyThatWillChange->GetFName() == GET_MEMBER_NAME_CHECKED(UMultiShapeComponent, CollisionShape))
	{
		//ShapeBodySetup = nullptr;
	}

	Super::PreEditChange(PropertyThatWillChange);
}
#endif // WITH_EDITOR