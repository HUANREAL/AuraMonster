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
	const int32 MaxAttempts = 30;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedOwner);
	
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Generate a random direction
		FVector RandomDirection = FMath::VRand();
		RandomDirection.Normalize();
		
		// Scale by range - use full range to reach distant surfaces
		float RandomDistance = FMath::RandRange(Range * 0.5f, Range);
		
		// Cast a ray from origin in the random direction to find surfaces
		FVector TraceStart = OriginLocation;
		FVector TraceEnd = OriginLocation + RandomDirection * RandomDistance;
		
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			// Found a surface along this ray
			OutLocation = HitResult.Location;
			OutNormal = HitResult.Normal;
			
			// Move the location slightly away from the surface to avoid being embedded
			OutLocation += OutNormal * 10.0f;
			
			return true;
		}
	}

	return false;
}

bool USurfacePathfindingComponent::MoveTowardsSurfaceLocation(const FVector& TargetLocation, float DeltaTime, float Speed)
{
	if (!CachedOwner || !GetWorld())
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
	float MovementThisFrame = FMath::Min(Speed * DeltaTime, DistanceToTarget);
	FVector DesiredLocation = CurrentLocation + DirectionToTarget * MovementThisFrame;

	// Instead of using DetectSurface which finds closest, do a directional trace
	// toward the target to find surfaces along the path
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedOwner);
	
	// First, try tracing toward the desired location
	FHitResult ForwardHit;
	bool bHitForward = GetWorld()->LineTraceSingleByChannel(
		ForwardHit, 
		CurrentLocation, 
		DesiredLocation + DirectionToTarget * SurfaceDetectionRange,
		ECC_Visibility, 
		QueryParams
	);
	
	if (bHitForward && ForwardHit.bBlockingHit)
	{
		// Check if hitting an obstacle (wall normal perpendicular to movement)
		// vs. hitting the surface we're moving along
		float DotWithMovement = FVector::DotProduct(ForwardHit.Normal, DirectionToTarget);
		
		// If the hit normal is pointing significantly against our movement direction,
		// it's likely an obstacle blocking our path
		if (DotWithMovement < -0.3f)
		{
			// This is an obstacle - don't move toward it
			// Instead, try to find a surface at our current position to stay grounded
			FVector NearestSurfaceLocation, NearestSurfaceNormal;
			if (DetectSurface(CurrentLocation, NearestSurfaceLocation, NearestSurfaceNormal))
			{
				CachedOwner->SetActorLocation(NearestSurfaceLocation);
				CurrentSurfaceNormal = NearestSurfaceNormal;
				bIsOnSurface = true;
				AlignToSurface(NearestSurfaceNormal, DeltaTime);
			}
			// Return true to indicate still trying (stuck detection in AI controller will handle this)
			return true;
		}
		else
		{
			// Hit a surface we can move onto (like floor or ceiling)
			FVector SurfaceLocation = ForwardHit.Location + ForwardHit.Normal * 10.0f;
			FVector SurfaceNormal = ForwardHit.Normal;
			
			CachedOwner->SetActorLocation(SurfaceLocation);
			CurrentSurfaceNormal = SurfaceNormal;
			bIsOnSurface = true;
			
			// Align to new surface
			AlignToSurface(SurfaceNormal, DeltaTime);
		}
	}
	else
	{
		// No surface hit forward, try detecting surface nearby
		FVector NearestSurfaceLocation, NearestSurfaceNormal;
		if (DetectSurface(DesiredLocation, NearestSurfaceLocation, NearestSurfaceNormal))
		{
			// Snap to detected surface
			CachedOwner->SetActorLocation(NearestSurfaceLocation);
			CurrentSurfaceNormal = NearestSurfaceNormal;
			bIsOnSurface = true;
			
			// Align to new surface
			AlignToSurface(NearestSurfaceNormal, DeltaTime);
		}
		else
		{
			// No surface found, just move normally
			CachedOwner->SetActorLocation(DesiredLocation);
			bIsOnSurface = false;
		}
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

	// Find surfaces and score them based on distance and alignment with current normal
	float BestScore = -1.0f;
	bool bFoundSurface = false;

	for (const FVector& Direction : TraceDirections)
	{
		FVector TraceStart = Location;
		FVector TraceEnd = Location + Direction * SurfaceDetectionRange;

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			float HitDistance = (HitResult.Location - Location).Size();
			
			// Score based on distance (closer is better) and alignment with current surface
			// This helps maintain continuity when moving along surfaces
			float DistanceScore = 1.0f - (HitDistance / SurfaceDetectionRange);
			
			// If we're on a surface, prefer surfaces that are similar to current orientation
			float AlignmentScore = 0.5f; // Neutral score if no current surface
			if (bIsOnSurface)
			{
				float DotProduct = FVector::DotProduct(CurrentSurfaceNormal, HitResult.Normal);
				// Positive dot = similar orientation, negative = opposite
				AlignmentScore = (DotProduct + 1.0f) * 0.5f; // Map [-1,1] to [0,1]
			}
			
			// Combined score: 70% distance, 30% alignment
			float Score = (DistanceScore * 0.7f) + (AlignmentScore * 0.3f);
			
			if (Score > BestScore)
			{
				BestScore = Score;
				OutHitLocation = HitResult.Location + HitResult.Normal * 10.0f; // Offset from surface
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
	
	// Get current forward direction and ensure it's normalized
	FVector CurrentForward = CachedOwner->GetActorForwardVector();
	CurrentForward.Normalize();
	
	// Calculate a right vector that's perpendicular to the target normal
	// Use the current forward vector or a fallback if they're parallel
	FVector RightVector = FVector::CrossProduct(TargetNormal, CurrentForward);
	if (RightVector.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		// Current forward is parallel to target normal, use a different reference vector
		FVector ReferenceVector = FMath::Abs(TargetNormal.Z) < 0.9f ? FVector::UpVector : FVector::ForwardVector;
		RightVector = FVector::CrossProduct(TargetNormal, ReferenceVector);
	}
	RightVector.Normalize();
	
	// Calculate the forward vector that's perpendicular to both the normal and right vector
	FVector ForwardVector = FVector::CrossProduct(RightVector, TargetNormal);
	ForwardVector.Normalize();
	
	// Create a rotation from these orthogonal vectors
	FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, TargetNormal);

	// Smoothly interpolate to the target rotation
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SurfaceAlignmentSpeed);
	
	CachedOwner->SetActorRotation(NewRotation);
}

bool USurfacePathfindingComponent::ShouldAttemptSurfaceTransition() const
{
	// Use randomness to create unpredictable surface transitions
	return FMath::FRand() < SurfaceTransitionChance;
}
