// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterAIController.h"
#include "MonsterCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

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

	// Initialize crawling behavior parameters with reasonable defaults
	SurfaceTransitionChance = 0.3f;
	MinSurfaceTransitionInterval = 3.0f;
	MaxSurfaceTransitionInterval = 8.0f;
	MaxSurfaceAngle = 90.0f;
	SurfaceSearchDistance = 500.0f;
	MaxSurfaceSearchAttempts = 8;
	SurfaceTransitionSearchRatio = 0.5f;
	SurfaceTransitionAngleThreshold = 0.9f;

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
	TimeSinceSurfaceTransitionCheck = 0.0f;
	NextSurfaceTransitionCheckTime = 0.0f;
	CurrentCrawlingDestination = FVector::ZeroVector;
	bHasCrawlingDestination = false;
	CachedSurfaceOffsetDistance = 50.0f;
	
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
	
	// Cache surface offset distance from controlled monster for performance
	if (ControlledMonster)
	{
		CachedSurfaceOffsetDistance = ControlledMonster->GetSurfaceOffsetDistance();
	}
	
	// Initialize NextSubtleMovementTime to prevent immediate trigger on first frame
	NextSubtleMovementTime = GetValidatedRandomRange(MinSubtleMovementInterval, MaxSubtleMovementInterval);
	
	// Initialize with idle state
	if (ControlledMonster)
	{
		TransitionToState(EMonsterBehaviorState::Idle);
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

		// Update monster character's state
		if (ControlledMonster)
		{
			ControlledMonster->SetBehaviorState(NewState);
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

	// Update surface transition timer
	TimeSinceSurfaceTransitionCheck += DeltaTime;

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
			bHasCrawlingDestination = false; // Force new destination selection
		}
		else
		{
			// Still waiting, don't move yet
			return;
		}
	}

	// Attempt unpredictable surface transition mid-patrol (but not while stopped)
	if (!bIsStoppedAtDestination && TimeSinceSurfaceTransitionCheck >= NextSurfaceTransitionCheckTime)
	{
		AttemptSurfaceTransition();
		TimeSinceSurfaceTransitionCheck = 0.0f;
		NextSurfaceTransitionCheckTime = GetValidatedRandomRange(MinSurfaceTransitionInterval, MaxSurfaceTransitionInterval);
	}

	// Check if we're currently moving to a destination
	if (bHasCrawlingDestination)
	{
		FVector CurrentLocation = ControlledMonster->GetActorLocation();
		float DistanceToDestination = FVector::Dist(CurrentLocation, CurrentCrawlingDestination);

		// Check if we've reached the destination
		if (DistanceToDestination <= PatrolAcceptanceRadius)
		{
			// We've reached destination, now stop to listen/look around
			bIsStoppedAtDestination = true;
			CurrentStopTime = 0.0f;
			TargetStopDuration = GetValidatedRandomRange(MinStopDuration, MaxStopDuration);
			bHasCrawlingDestination = false;
			
			// Stop movement
			StopMovement();
			return;
		}

		// Continue moving toward destination
		// Project movement direction onto the surface plane to ensure surface-relative movement
		FVector DirectionToDestination = (CurrentCrawlingDestination - CurrentLocation).GetSafeNormal();
		FVector SurfaceNormal = ControlledMonster->GetCurrentSurfaceNormal();
		FVector ProjectedDirection = FVector::VectorPlaneProject(DirectionToDestination, SurfaceNormal).GetSafeNormal();
		
		// Apply movement input
		if (ControlledMonster->GetCharacterMovement())
		{
			ControlledMonster->AddMovementInput(ProjectedDirection, 1.0f);
		}
		
		return;
	}

	// Need to find a new crawling destination on a surface
	FVector NewDestination;
	if (FindCrawlingSurfaceDestination(NewDestination))
	{
		CurrentCrawlingDestination = NewDestination;
		bHasCrawlingDestination = true;
	}
}

bool AMonsterAIController::FindCrawlingSurfaceDestination(FVector& OutDestination)
{
	if (!ControlledMonster || !GetWorld())
	{
		return false;
	}

	FVector CurrentLocation = ControlledMonster->GetActorLocation();
	
	// Compute trace directions relative to the monster's current orientation
	const FVector TraceDirections[] = {
		-ControlledMonster->GetActorUpVector(),    // Down relative to monster
		ControlledMonster->GetActorForwardVector(), // Forward
		ControlledMonster->GetActorUpVector(),      // Up relative to monster
		ControlledMonster->GetActorRightVector(),   // Right
		-ControlledMonster->GetActorRightVector(),  // Left
		-ControlledMonster->GetActorForwardVector() // Backward
	};

	// Try multiple random directions to find a valid surface location
	for (int32 Attempt = 0; Attempt < MaxSurfaceSearchAttempts; ++Attempt)
	{
		// Generate a random direction
		FVector RandomDirection = FMath::VRand();
		
		// Generate search location (surface tracing will validate actual locations)
		FVector SearchLocation = CurrentLocation + RandomDirection * SurfaceSearchDistance;
		
		// Trace to find surfaces in multiple directions
		for (const FVector& TraceDir : TraceDirections)
		{
			FVector TraceStart = SearchLocation;
			FVector TraceEnd = SearchLocation + TraceDir * SurfaceSearchDistance;
			
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(ControlledMonster);
			
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				TraceStart,
				TraceEnd,
				ECC_Visibility,
				QueryParams
			);
			
			if (bHit)
			{
				// Check if the surface angle is within acceptable range
				// Clamp dot product to prevent NaN from floating-point precision errors
				float DotProduct = FMath::Clamp(FVector::DotProduct(HitResult.ImpactNormal, FVector::UpVector), -1.0f, 1.0f);
				float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
				
				if (SurfaceAngle <= MaxSurfaceAngle)
				{
					// Valid surface found - offset slightly from surface using cached value
					OutDestination = HitResult.ImpactPoint + HitResult.ImpactNormal * CachedSurfaceOffsetDistance;
					return true; // Early exit when valid destination found
				}
			}
		}
	}
	
	// Fallback: use standard navigation if no surface found
	if (CachedNavSystem)
	{
		FNavLocation ResultLocation;
		bool bFoundLocation = CachedNavSystem->GetRandomReachablePointInRadius(CurrentLocation, PatrolRange, ResultLocation);
		
		if (bFoundLocation)
		{
			OutDestination = ResultLocation.Location;
			return true;
		}
	}
	
	return false;
}

void AMonsterAIController::AttemptSurfaceTransition()
{
	// Check if we should attempt a surface transition
	float RandomValue = FMath::FRand();
	if (RandomValue >= SurfaceTransitionChance)
	{
		return; // No transition this time
	}
	
	if (!ControlledMonster || !GetWorld())
	{
		return;
	}
	
	FVector CurrentLocation = ControlledMonster->GetActorLocation();
	FVector CurrentUpVector = ControlledMonster->GetActorUpVector();
	
	// Calculate search directions dynamically based on current character orientation
	// to look for adjacent surfaces at different angles
	FVector RightVector = ControlledMonster->GetActorRightVector();
	FVector ForwardVector = ControlledMonster->GetActorForwardVector();
	
	// Search in perpendicular directions for wall/ceiling transitions
	const FVector SearchDirections[] = {
		RightVector,         // Right
		-RightVector,        // Left
		ForwardVector,       // Forward
		-ForwardVector,      // Backward
		-CurrentUpVector     // Down (opposite to up)
	};
	
	for (const FVector& SearchDir : SearchDirections)
	{
		FVector TraceStart = CurrentLocation;
		FVector TraceEnd = CurrentLocation + SearchDir * SurfaceSearchDistance * SurfaceTransitionSearchRatio;
		
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(ControlledMonster);
		
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);
		
		if (bHit)
		{
			// Check if this is a different surface
			// Clamp dot product to prevent precision errors
			float DotProduct = FMath::Clamp(FVector::DotProduct(CurrentUpVector, HitResult.ImpactNormal), -1.0f, 1.0f);
			
			// If surface normal is significantly different, transition to it
			// Compare raw dot product to detect genuinely different orientations
			if (DotProduct < SurfaceTransitionAngleThreshold)
			{
				// Set new destination on the different surface using cached offset value
				CurrentCrawlingDestination = HitResult.ImpactPoint + HitResult.ImpactNormal * CachedSurfaceOffsetDistance;
				bHasCrawlingDestination = true;
				bIsStoppedAtDestination = false; // Cancel any current stop
				CurrentStopTime = 0.0f; // Reset stop timer for clean state
				return; // Successfully found and set transition destination
			}
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
		
		// Reset crawling-specific variables if entering crawling state
		if (NewState == EMonsterBehaviorState::PatrolCrawling)
		{
			TimeSinceSurfaceTransitionCheck = 0.0f;
			NextSurfaceTransitionCheckTime = GetValidatedRandomRange(MinSurfaceTransitionInterval, MaxSurfaceTransitionInterval);
			bHasCrawlingDestination = false;
			
			// Refresh cached surface offset distance
			if (ControlledMonster)
			{
				CachedSurfaceOffsetDistance = ControlledMonster->GetSurfaceOffsetDistance();
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
