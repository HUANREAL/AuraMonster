// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurfacePathfindingComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

USurfacePathfindingComponent::USurfacePathfindingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize default values
	SurfaceTransitionChance = 0.3f;
	SurfaceDetectionRange = 200.0f;
	SurfaceAlignmentSpeed = 5.0f;
	MinTransitionAngle = 45.0f;
	AcceptanceRadius = 100.0f;

	CurrentSurfaceNormal = FVector::UpVector;
	bIsOnSurface = false;
	CachedOwner = nullptr;
}

void USurfacePathfindingComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwner = GetOwner();
	
	// Initialize current surface by detecting ground
	if (CachedOwner)
	{
		FVector HitLocation, HitNormal;
		if (DetectSurface(CachedOwner->GetActorLocation(), HitLocation, HitNormal))
		{
			CurrentSurfaceNormal = HitNormal;
			bIsOnSurface = true;
		}
	}
}

void USurfacePathfindingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Continuously update surface attachment
	if (CachedOwner && bIsOnSurface)
	{
		FVector HitLocation, HitNormal;
		if (DetectSurface(CachedOwner->GetActorLocation(), HitLocation, HitNormal))
		{
			CurrentSurfaceNormal = HitNormal;
			AlignToSurface(HitNormal, DeltaTime);
		}
		else
		{
			bIsOnSurface = false;
		}
	}
}

bool USurfacePathfindingComponent::GetRandomSurfaceLocation(const FVector& OriginLocation, float Range, FVector& OutLocation, FVector& OutNormal)
{
	if (!CachedOwner || !GetWorld())
	{
		return false;
	}

	// Try multiple random directions to find a valid surface location
	const int32 MaxAttempts = 20;
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Generate a random direction
		FVector RandomDirection = FMath::VRand();
		
		// Scale by range
		FVector RandomOffset = RandomDirection * FMath::RandRange(Range * 0.3f, Range);
		FVector TestLocation = OriginLocation + RandomOffset;

		// Try to find a surface near this test location
		FVector HitLocation, HitNormal;
		if (DetectSurface(TestLocation, HitLocation, HitNormal))
		{
			OutLocation = HitLocation;
			OutNormal = HitNormal;
			return true;
		}
	}

	return false;
}

bool USurfacePathfindingComponent::MoveTowardsSurfaceLocation(const FVector& TargetLocation, float DeltaTime, float Speed)
{
	if (!CachedOwner)
	{
		return false;
	}

	FVector CurrentLocation = CachedOwner->GetActorLocation();
	FVector DirectionToTarget = TargetLocation - CurrentLocation;
	float DistanceToTarget = DirectionToTarget.Size();

	// Check if we've reached the target
	if (DistanceToTarget <= AcceptanceRadius)
	{
		return false; // Reached target
	}

	// Normalize direction
	DirectionToTarget.Normalize();

	// Calculate movement for this frame
	float MovementThisFrame = Speed * DeltaTime;
	FVector NewLocation = CurrentLocation + DirectionToTarget * MovementThisFrame;

	// Detect surface at new location and adjust to stay on surface
	FVector SurfaceLocation, SurfaceNormal;
	if (DetectSurface(NewLocation, SurfaceLocation, SurfaceNormal))
	{
		// Snap to surface
		CachedOwner->SetActorLocation(SurfaceLocation);
		CurrentSurfaceNormal = SurfaceNormal;
		bIsOnSurface = true;

		// Align to new surface
		AlignToSurface(SurfaceNormal, DeltaTime);

		// Check if we should attempt a surface transition for unpredictability
		if (ShouldAttemptSurfaceTransition())
		{
			// Calculate angle between current and new surface
			float AngleDifference = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurrentSurfaceNormal, SurfaceNormal)));
			
			// Only transition if angle difference is significant
			if (AngleDifference >= MinTransitionAngle)
			{
				// This creates the unpredictable surface-switching behavior
				CurrentSurfaceNormal = SurfaceNormal;
			}
		}
	}
	else
	{
		// No surface found, just move normally
		CachedOwner->SetActorLocation(NewLocation);
		bIsOnSurface = false;
	}

	return true; // Still moving
}

bool USurfacePathfindingComponent::IsOnValidSurface() const
{
	return bIsOnSurface;
}

bool USurfacePathfindingComponent::DetectSurface(const FVector& Location, FVector& OutHitLocation, FVector& OutHitNormal)
{
	if (!GetWorld())
	{
		return false;
	}

	// Perform multi-directional traces to detect surfaces in all directions
	// This allows detection of floors, walls, and ceilings
	
	const FVector TraceDirections[] = {
		FVector(0, 0, -1),  // Down (floor)
		FVector(0, 0, 1),   // Up (ceiling)
		FVector(1, 0, 0),   // Right (wall)
		FVector(-1, 0, 0),  // Left (wall)
		FVector(0, 1, 0),   // Forward (wall)
		FVector(0, -1, 0)   // Backward (wall)
	};

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedOwner);

	// Find the closest surface in any direction
	float ClosestDistance = SurfaceDetectionRange + 1.0f;
	bool bFoundSurface = false;

	for (const FVector& Direction : TraceDirections)
	{
		FVector TraceStart = Location;
		FVector TraceEnd = Location + Direction * SurfaceDetectionRange;

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			float HitDistance = (HitResult.Location - Location).Size();
			if (HitDistance < ClosestDistance)
			{
				ClosestDistance = HitDistance;
				OutHitLocation = HitResult.Location;
				OutHitNormal = HitResult.Normal;
				bFoundSurface = true;
			}
		}
	}

	return bFoundSurface;
}

void USurfacePathfindingComponent::AlignToSurface(const FVector& TargetNormal, float DeltaTime)
{
	if (!CachedOwner)
	{
		return;
	}

	// Calculate the target rotation that aligns the actor's up vector with the surface normal
	FRotator CurrentRotation = CachedOwner->GetActorRotation();
	
	// Create a rotation where the Z-axis (up) aligns with the surface normal
	FRotator TargetRotation = UKismetMathLibrary::MakeRotFromZX(TargetNormal, CachedOwner->GetActorForwardVector());

	// Smoothly interpolate to the target rotation
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SurfaceAlignmentSpeed);
	
	CachedOwner->SetActorRotation(NewRotation);
}

bool USurfacePathfindingComponent::ShouldAttemptSurfaceTransition() const
{
	// Use randomness to create unpredictable surface transitions
	return FMath::FRand() < SurfaceTransitionChance;
}
