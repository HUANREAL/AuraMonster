// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterAIController.h"
#include "MonsterCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

// Initialize static array of fallback trace directions for surface detection
const FVector AMonsterAIController::FallbackTraceDirections[6] = {
	FVector::UpVector,
	FVector::DownVector,
	FVector::ForwardVector,
	FVector::BackwardVector,
	FVector::RightVector,
	FVector::LeftVector
};

AMonsterAIController::AMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentState = EMonsterBehaviorState::Idle;
	ControlledMonster = nullptr;

	// Initialize idle behavior parameters with reasonable defaults
	MinIdleDuration = 5.0f;
	MaxIdleDuration = 15.0f;
	MinSubtleMovementInterval = 2.0f;
	MaxSubtleMovementInterval = 6.0f;
	BreathingCycleDuration = 4.0f;
	PatrolTransitionChance = 0.3f;

	// Initialize patrol behavior parameters with reasonable defaults
	PatrolRange = 1000.0f;
	MinStopDuration = 2.0f;
	MaxStopDuration = 5.0f;
	PatrolAcceptanceRadius = 100.0f;

	// Initialize crawling behavior parameters
	CrawlSurfaceDetectionDistance = 2000.0f;
	SurfaceTransitionChance = 0.3f;
	SurfaceAlignmentSpeed = 5.0f;
	CrawlSurfaceOffset = 50.0f;
	FallbackTraceUpDistance = 100.0f;
	FallbackTraceDownDistance = 500.0f;
	MinCrawlPitch = -45.0f;
	MaxCrawlPitch = 45.0f;
	MinTransitionPitch = -75.0f;
	MaxTransitionPitch = 75.0f;
	MinCrawlDistanceMultiplier = 0.3f;
	SurfaceTraceDistanceMultiplier = 0.5f;
	MovementDirectionBlendSpeed = 2.5f;

	// Initialize timing variables
	CurrentIdleTime = 0.0f;
	TargetIdleDuration = FMath::RandRange(MinIdleDuration, MaxIdleDuration);
	TimeSinceLastSubtleMovement = 0.0f;
	NextSubtleMovementTime = 0.0f;
	BreathingCycleTime = 0.0f;
	
	// Initialize patrol variables
	CurrentStopTime = 0.0f;
	TargetStopDuration = 0.0f;
	bIsStoppedAtDestination = false;

	// Initialize crawling variables
	CurrentSurfaceNormal = FVector::UpVector;
	TargetSurfaceNormal = FVector::UpVector;
	CrawlingDestination = FVector::ZeroVector;
	bIsMovingToCrawlDestination = false;
	bIsTransitioningBetweenSurfaces = false;
	CurrentCrawlStopTime = 0.0f;
	TargetCrawlStopDuration = 0.0f;
	bIsStoppedAtCrawlDestination = false;
	
	// Initialize cached references
	CachedNavSystem = nullptr;
	CachedPathFollowingComp = nullptr;
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();

	// Cache reference to the controlled monster
	ControlledMonster = Cast<AMonsterCharacter>(GetPawn());
	
	// Cache navigation system reference
	CachedNavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	
	// Cache path following component reference
	CachedPathFollowingComp = GetPathFollowingComponent();
	
	// Initialize NextSubtleMovementTime to prevent immediate trigger on first frame
	NextSubtleMovementTime = GetValidatedRandomRange(MinSubtleMovementInterval, MaxSubtleMovementInterval);
	
	// Initialize with current state (respects pre-configured state from editor)
	if (ControlledMonster)
	{
		// Use internal method to set character state without triggering AI Controller sync
		// This avoids unnecessary circular logic during initialization
		ControlledMonster->SetBehaviorStateInternal(CurrentState);
		
		// Initialize state-specific variables by calling OnEnterState
		OnEnterState(CurrentState);
	}
}

void AMonsterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Execute behavior based on current state
	switch (CurrentState)
	{
		case EMonsterBehaviorState::Idle:
			ExecuteIdleBehavior(DeltaTime);
			break;

		case EMonsterBehaviorState::PatrolStanding:
			ExecutePatrolStandingBehavior(DeltaTime);
			break;

		case EMonsterBehaviorState::PatrolCrawling:
			ExecutePatrolCrawlingBehavior(DeltaTime);
			break;
	}
}

void AMonsterAIController::TransitionToState(EMonsterBehaviorState NewState)
{
	if (CurrentState != NewState)
	{
		// Exit old state
		OnExitState(CurrentState);

		EMonsterBehaviorState OldState = CurrentState;
		CurrentState = NewState;

		// Update monster character's state using internal method to avoid circular synchronization
		if (ControlledMonster)
		{
			ControlledMonster->SetBehaviorStateInternal(NewState);
		}

		// Enter new state
		OnEnterState(NewState);
	}
}

void AMonsterAIController::ExecuteIdleBehavior_Implementation(float DeltaTime)
{
	if (!ControlledMonster)
	{
		return;
	}

	// Update idle time
	CurrentIdleTime += DeltaTime;

	// Update breathing cycle - only if BreathingCycleDuration is valid
	if (BreathingCycleDuration > 0.0f)
	{
		BreathingCycleTime += DeltaTime;
		BreathingCycleTime = FMath::Fmod(BreathingCycleTime, BreathingCycleDuration);

		// Calculate breathing intensity (sine wave for smooth breathing)
		// Multiply by 2*PI to convert normalized time (0-1) to radians for full sine wave cycle
		const float NormalizedTime = BreathingCycleTime / BreathingCycleDuration;
		float BreathingIntensity = (FMath::Sin(NormalizedTime * 2.0f * PI) + 1.0f) * 0.5f;
		ControlledMonster->OnBreathingUpdate(BreathingIntensity);
	}

	// Handle subtle random movements
	TimeSinceLastSubtleMovement += DeltaTime;
	if (TimeSinceLastSubtleMovement >= NextSubtleMovementTime)
	{
		// Trigger a random subtle movement
		float RandomValue = FMath::FRand();
		if (RandomValue < 0.5f)
		{
			// Neck twitch
			ControlledMonster->OnNeckTwitch();
		}
		else
		{
			// Finger shift
			ControlledMonster->OnFingerShift();
		}

		// Reset timer and set next movement time
		TimeSinceLastSubtleMovement = 0.0f;
		NextSubtleMovementTime = GetValidatedRandomRange(MinSubtleMovementInterval, MaxSubtleMovementInterval);
	}

	// Check if should transition to patrol
	if (CurrentIdleTime >= TargetIdleDuration)
	{
		// Decide whether to patrol or stay idle
		float RandomValue = FMath::FRand();
		if (RandomValue < PatrolTransitionChance)
		{
			// Randomly choose between standing and crawling patrol
			EMonsterBehaviorState NewState = (FMath::FRand() < 0.5f) 
				? EMonsterBehaviorState::PatrolStanding 
				: EMonsterBehaviorState::PatrolCrawling;
			TransitionToState(NewState);
		}
		else
		{
			// Stay idle but reset the idle duration
			CurrentIdleTime = 0.0f;
			TargetIdleDuration = GetValidatedRandomRange(MinIdleDuration, MaxIdleDuration);
		}
	}
}

void AMonsterAIController::ExecutePatrolStandingBehavior_Implementation(float DeltaTime)
{
	if (!ControlledMonster)
	{
		return;
	}

	// Check if we're currently stopped at a destination to listen/look around
	if (bIsStoppedAtDestination)
	{
		CurrentStopTime += DeltaTime;
		
		// Check if we've waited long enough
		if (CurrentStopTime >= TargetStopDuration)
		{
			// Done stopping, ready to move to next destination
			bIsStoppedAtDestination = false;
			CurrentStopTime = 0.0f;
			// Fall through to select new destination
		}
		else
		{
			// Still waiting, don't move yet
			return;
		}
	}

	// Check if we're currently moving to a destination using cached component
	if (CachedPathFollowingComp)
	{
		// Check current path following status
		EPathFollowingStatus::Type Status = CachedPathFollowingComp->GetStatus();

		// Only check DidMoveReachGoal if currently moving
		if (Status == EPathFollowingStatus::Moving)
		{
			// Check if we've reached the current destination
			if (CachedPathFollowingComp->DidMoveReachGoal())
			{
				// We've reached destination, now stop to listen/look around
				bIsStoppedAtDestination = true;
				CurrentStopTime = 0.0f;
				TargetStopDuration = GetValidatedRandomRange(MinStopDuration, MaxStopDuration);
				
				// Stop movement
				StopMovement();
				return;
			}

			// Still moving to current destination, continue
			return;
		}
		
		// If the status is not Idle, do not select a new destination
		// This prevents rapid destination changes during transient states
		if (Status != EPathFollowingStatus::Idle)
		{
			// For Paused, Waiting, Aborting, etc. - wait for status to settle
			return;
		}
	}

	// Need to select a new random patrol destination using cached navigation system
	if (!CachedNavSystem)
	{
		return;
	}

	// Get current location
	FVector CurrentLocation = ControlledMonster->GetActorLocation();
	
	// Try to find a random reachable point within patrol range
	FNavLocation ResultLocation;
	bool bFoundLocation = CachedNavSystem->GetRandomReachablePointInRadius(CurrentLocation, PatrolRange, ResultLocation);
	
	if (bFoundLocation)
	{
		// Move to the new patrol destination with deliberate, heavy pace
		// The movement speed is configured via PatrolStandingSpeed property in AMonsterCharacter
		MoveToLocation(ResultLocation.Location, PatrolAcceptanceRadius);
	}
	else
	{
		// Failed to find a reachable location, try again with a smaller radius
		FNavLocation CloserResultLocation;
		bool bFoundCloserLocation = CachedNavSystem->GetRandomReachablePointInRadius(CurrentLocation, PatrolRange * 0.5f, CloserResultLocation);
		if (bFoundCloserLocation)
		{
			MoveToLocation(CloserResultLocation.Location, PatrolAcceptanceRadius);
		}
		// If still can't find a location, the monster will try again on the next tick
		// This prevents getting stuck while allowing for environmental constraints
	}
}

void AMonsterAIController::ExecutePatrolCrawlingBehavior_Implementation(float DeltaTime)
{
	if (!ControlledMonster)
	{
		return;
	}

	// Update surface alignment for smooth transitions
	UpdateSurfaceAlignment(DeltaTime);

	// Check if we're currently stopped at a destination to listen/look around
	if (bIsStoppedAtCrawlDestination)
	{
		CurrentCrawlStopTime += DeltaTime;
		
		// Check if we've waited long enough
		if (CurrentCrawlStopTime >= TargetCrawlStopDuration)
		{
			// Done stopping, ready to move to next destination
			bIsStoppedAtCrawlDestination = false;
			CurrentCrawlStopTime = 0.0f;
			bIsMovingToCrawlDestination = false;
		}
		else
		{
			// Still waiting, don't move yet
			return;
		}
	}

	// Check if we're currently moving to a crawl destination
	if (bIsMovingToCrawlDestination)
	{
		// Check if we've reached the destination
		if (HasReachedCrawlDestination())
		{
			// We've reached destination, now stop to listen/look around
			bIsStoppedAtCrawlDestination = true;
			bIsMovingToCrawlDestination = false;
			CurrentCrawlStopTime = 0.0f;
			TargetCrawlStopDuration = GetValidatedRandomRange(MinStopDuration, MaxStopDuration);
			
			// Potentially transition to a different surface
			// Set flag to bias pathfinding towards different surface orientations (walls/ceilings)
			if (FMath::FRand() < SurfaceTransitionChance)
			{
				// Mark that we should look for a surface with a different orientation
				// This biases the pitch range in FindCrawlableDestination to favor vertical surfaces
				bIsTransitioningBetweenSurfaces = true;
			}
			
			return;
		}
		
		// Move towards the crawling destination with surface-aware movement
		FVector CurrentLocation = ControlledMonster->GetActorLocation();
		
		// Calculate movement direction towards destination
		FVector ToDestination = (CrawlingDestination - CurrentLocation).GetSafeNormal();
		
		// Get movement speed for crawling state
		float MovementSpeed = ControlledMonster->GetMovementSpeedForState(EMonsterBehaviorState::PatrolCrawling);
		float MovementDistance = MovementSpeed * DeltaTime;
		
		// Calculate the next position
		FVector NextPosition = CurrentLocation + ToDestination * MovementDistance;
		
		// Calculate trace distance for surface detection
		const float TraceDistance = CrawlSurfaceDetectionDistance * SurfaceTraceDistanceMultiplier;
		
		// Trace to find the surface beneath/beside the new position
		// This allows the monster to continuously follow the surface contours
		FVector SurfaceCheckNormal = CurrentSurfaceNormal;
		FVector TraceStart = NextPosition + SurfaceCheckNormal * TraceDistance;
		FVector TraceEnd = NextPosition - SurfaceCheckNormal * TraceDistance;
		
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(ControlledMonster);
		
		// Lambda to update position on detected surface
		// Capture only the specific variables needed to avoid unintended side effects
		auto UpdatePositionOnSurface = [this, &TargetSurfaceNormal](const FHitResult& Hit)
		{
			FVector SurfacePoint = Hit.ImpactPoint;
			FVector SurfaceNormal = Hit.ImpactNormal;
			
			// Update position to be on the surface with proper offset
			FVector NewLocation = SurfacePoint + SurfaceNormal * CrawlSurfaceOffset;
			ControlledMonster->SetActorLocation(NewLocation);
			
			// Update target surface normal for smooth alignment
			TargetSurfaceNormal = SurfaceNormal;
		};
		
		// Trace to find the actual surface at the new position
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			// Found a surface, stick to it
			UpdatePositionOnSurface(HitResult);
		}
		else
		{
			// No surface found with current orientation, try multi-directional trace
			// This helps when transitioning between faces of a column or complex geometry
			// Note: This fallback is only used when the primary trace fails, keeping overhead minimal
			bool bFoundSurface = false;
			
			// Try tracing in multiple directions using pre-allocated static array
			// The loop will early exit as soon as a surface is found (break on line 432)
			const int32 NumTraceDirections = UE_ARRAY_COUNT(FallbackTraceDirections);
			for (int32 i = 0; i < NumTraceDirections; ++i)
			{
				const FVector& TraceDir = FallbackTraceDirections[i];
				FVector MultiTraceStart = NextPosition + TraceDir * TraceDistance;
				FVector MultiTraceEnd = NextPosition - TraceDir * TraceDistance;
				
				if (GetWorld()->LineTraceSingleByChannel(HitResult, MultiTraceStart, MultiTraceEnd, ECC_Visibility, QueryParams))
				{
					// Found a surface - early exit to minimize traces
					UpdatePositionOnSurface(HitResult);
					bFoundSurface = true;
					break;
				}
			}
			
			// If still no surface found, move without surface constraint
			// This can happen when moving through open space between surfaces
			if (!bFoundSurface)
			{
				ControlledMonster->SetActorLocation(NextPosition);
			}
		}
		
		return;
	}

	// Need to select a new crawl destination
	FVector NewDestination;
	FVector NewSurfaceNormal;
	
	if (FindCrawlableDestination(NewDestination, NewSurfaceNormal))
	{
		// Found a valid crawlable destination
		CrawlingDestination = NewDestination;
		TargetSurfaceNormal = NewSurfaceNormal;
		bIsMovingToCrawlDestination = true;
		bIsTransitioningBetweenSurfaces = false;
		
		// Orient towards the destination
		FVector Direction = (CrawlingDestination - ControlledMonster->GetActorLocation()).GetSafeNormal();
		if (!Direction.IsNearlyZero())
		{
			FRotator TargetRotation = Direction.Rotation();
			ControlledMonster->SetActorRotation(TargetRotation);
		}
	}
}

void AMonsterAIController::OnEnterState_Implementation(EMonsterBehaviorState NewState)
{
	// Initialize state-specific variables when entering a state
	if (NewState == EMonsterBehaviorState::Idle)
	{
		// Validate breathing cycle duration to avoid division by zero
		if (BreathingCycleDuration <= 0.0f)
		{
			BreathingCycleDuration = 4.0f;
		}
		
		// Reset idle timing
		CurrentIdleTime = 0.0f;
		
		// Validate idle duration range
		TargetIdleDuration = GetValidatedRandomRange(MinIdleDuration, MaxIdleDuration);
		
		// Reset subtle movement timing
		TimeSinceLastSubtleMovement = 0.0f;
		
		// Validate subtle movement interval range
		NextSubtleMovementTime = GetValidatedRandomRange(MinSubtleMovementInterval, MaxSubtleMovementInterval);
		
		// Reset breathing cycle
		BreathingCycleTime = 0.0f;
	}
	else if (NewState == EMonsterBehaviorState::PatrolStanding || NewState == EMonsterBehaviorState::PatrolCrawling)
	{
		// Reset patrol timing variables
		CurrentStopTime = 0.0f;
		TargetStopDuration = 0.0f;
		bIsStoppedAtDestination = false;
		
		// Reset crawling-specific variables
		if (NewState == EMonsterBehaviorState::PatrolCrawling)
		{
			CurrentCrawlStopTime = 0.0f;
			TargetCrawlStopDuration = 0.0f;
			bIsStoppedAtCrawlDestination = false;
			bIsMovingToCrawlDestination = false;
			bIsTransitioningBetweenSurfaces = false;
			
			// Initialize surface normal to current floor
			if (ControlledMonster)
			{
				FVector Location = ControlledMonster->GetActorLocation();
				FVector TraceStart = Location + FVector(0.0f, 0.0f, FallbackTraceUpDistance);
				FVector TraceEnd = Location - FVector(0.0f, 0.0f, FallbackTraceDownDistance);
				
				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(ControlledMonster);
				
				if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
				{
					CurrentSurfaceNormal = HitResult.ImpactNormal;
					TargetSurfaceNormal = HitResult.ImpactNormal;
				}
				else
				{
					CurrentSurfaceNormal = FVector::UpVector;
					TargetSurfaceNormal = FVector::UpVector;
				}
			}
		}
	}
}

void AMonsterAIController::OnExitState_Implementation(EMonsterBehaviorState OldState)
{
	// Called when exiting a state
	// Can be overridden to clean up state-specific logic
}

float AMonsterAIController::GetValidatedRandomRange(float MinValue, float MaxValue) const
{
	// Ensure MinValue <= MaxValue by using FMath::Min/Max
	float ValidMin = FMath::Min(MinValue, MaxValue);
	float ValidMax = FMath::Max(MinValue, MaxValue);
	return FMath::RandRange(ValidMin, ValidMax);
}

bool AMonsterAIController::FindCrawlableDestination(FVector& OutLocation, FVector& OutSurfaceNormal)
{
	if (!ControlledMonster || !GetWorld())
	{
		return false;
	}

	FVector CurrentLocation = ControlledMonster->GetActorLocation();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ControlledMonster);

	// Maximum number of attempts to find a crawlable surface
	const int32 MaxAttempts = 8;
	
	// Try multiple random directions to find a crawlable surface
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Generate a random direction
		// Bias towards forward and sides for more natural movement
		float Yaw = FMath::RandRange(-180.0f, 180.0f);
		float Pitch = FMath::RandRange(MinCrawlPitch, MaxCrawlPitch);
		
		// If transitioning between surfaces, bias towards different orientations (walls/ceilings)
		if (bIsTransitioningBetweenSurfaces)
		{
			// Increase pitch range to favor vertical surfaces
			Pitch = FMath::RandRange(MinTransitionPitch, MaxTransitionPitch);
		}
		
		FRotator RandomRotation(Pitch, Yaw, 0.0f);
		FVector RandomDirection = RandomRotation.Vector();

		// Random distance within patrol range using configurable minimum
		float Distance = FMath::RandRange(PatrolRange * MinCrawlDistanceMultiplier, PatrolRange);
		FVector TargetPoint = CurrentLocation + RandomDirection * Distance;

		// Trace towards the target point to find a surface
		// Use the random direction perpendicular to properly detect walls and ceilings
		// The trace extends in both directions (0.5 * detection distance each way)
		FVector TraceStart = TargetPoint + RandomDirection * (CrawlSurfaceDetectionDistance * 0.5f);
		FVector TraceEnd = TargetPoint - RandomDirection * (CrawlSurfaceDetectionDistance * 0.5f);

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			// Found a surface, check if it's accessible
			FVector SurfaceLocation = HitResult.ImpactPoint;
			FVector SurfaceNormal = HitResult.ImpactNormal;

			// Offset slightly from the surface using configurable offset
			SurfaceLocation += SurfaceNormal * CrawlSurfaceOffset;

			// Check if we can trace a path to this location
			FHitResult PathCheckHit;
			if (!GetWorld()->LineTraceSingleByChannel(PathCheckHit, CurrentLocation, SurfaceLocation, ECC_Visibility, QueryParams))
			{
				// No obstacles in the way
				OutLocation = SurfaceLocation;
				OutSurfaceNormal = SurfaceNormal;
				return true;
			}
		}

		// Try a different approach: trace in the random direction and then down/up to find nearest surface
		FVector DirectTraceEnd = CurrentLocation + RandomDirection * Distance;
		FHitResult DirectHit;
		if (GetWorld()->LineTraceSingleByChannel(DirectHit, CurrentLocation, DirectTraceEnd, ECC_Visibility, QueryParams))
		{
			// Hit something, trace perpendicular to find the surface
			FVector HitPoint = DirectHit.ImpactPoint;
			FVector HitNormal = DirectHit.ImpactNormal;
			
			// Offset from the surface using configurable offset
			OutLocation = HitPoint + HitNormal * CrawlSurfaceOffset;
			OutSurfaceNormal = HitNormal;
			return true;
		}
	}

	// Fallback: try to find floor beneath current position using configurable distances
	FVector TraceStart = CurrentLocation + FVector(0.0f, 0.0f, FallbackTraceUpDistance);
	FVector TraceEnd = CurrentLocation - FVector(0.0f, 0.0f, FallbackTraceDownDistance);
	
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		OutLocation = HitResult.ImpactPoint + HitResult.ImpactNormal * CrawlSurfaceOffset;
		OutSurfaceNormal = HitResult.ImpactNormal;
		return true;
	}

	return false;
}

void AMonsterAIController::UpdateSurfaceAlignment(float DeltaTime)
{
	// Note: Setting SurfaceAlignmentSpeed to 0.0 or negative disables surface alignment
	if (!ControlledMonster || SurfaceAlignmentSpeed <= 0.0f)
	{
		return;
	}

	// Smoothly interpolate from current to target surface normal
	CurrentSurfaceNormal = FMath::VInterpTo(CurrentSurfaceNormal, TargetSurfaceNormal, DeltaTime, SurfaceAlignmentSpeed);
	
	// Calculate rotation to align with surface
	// The "up" vector should align with the surface normal
	FRotator CurrentRotation = ControlledMonster->GetActorRotation();
	FVector ForwardVector = CurrentRotation.Vector();
	
	// If moving towards a crawl destination, also consider the movement direction
	if (bIsMovingToCrawlDestination)
	{
		FVector ToDestination = (CrawlingDestination - ControlledMonster->GetActorLocation()).GetSafeNormal();
		if (!ToDestination.IsNearlyZero())
		{
			// Blend current forward with destination direction for smoother turning
			// Use configurable blend speed for tunable turning behavior
			ForwardVector = FMath::VInterpTo(ForwardVector, ToDestination, DeltaTime, MovementDirectionBlendSpeed);
		}
	}
	
	// Project forward vector onto the surface plane
	FVector ProjectedForward = ForwardVector - CurrentSurfaceNormal * FVector::DotProduct(ForwardVector, CurrentSurfaceNormal);
	ProjectedForward.Normalize();
	
	// Build a rotation from the projected forward and surface normal
	if (!ProjectedForward.IsNearlyZero() && !CurrentSurfaceNormal.IsNearlyZero())
	{
		FMatrix RotationMatrix = FRotationMatrix::MakeFromXZ(ProjectedForward, CurrentSurfaceNormal);
		FRotator TargetRotation = RotationMatrix.Rotator();
		
		// Smoothly rotate towards target
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SurfaceAlignmentSpeed);
		ControlledMonster->SetActorRotation(NewRotation);
	}
}

bool AMonsterAIController::HasReachedCrawlDestination() const
{
	if (!ControlledMonster)
	{
		return false;
	}

	FVector CurrentLocation = ControlledMonster->GetActorLocation();
	float Distance = FVector::Dist(CurrentLocation, CrawlingDestination);
	return Distance <= PatrolAcceptanceRadius;
}
