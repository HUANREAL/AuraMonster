// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterAIController.h"
#include "MonsterCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

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
		if (CachedPathFollowingComp->GetStatus() == EPathFollowingStatus::Moving)
		{
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
		bool bFoundCloserLocation = CachedNavSystem->GetRandomReachablePointInRadius(CurrentLocation, PatrolRange * 0.5f, ResultLocation);
		if (bFoundCloserLocation)
		{
			MoveToLocation(ResultLocation.Location, PatrolAcceptanceRadius);
		}
		// If still can't find a location, the monster will try again on the next tick
		// This prevents getting stuck while allowing for environmental constraints
	}
}

void AMonsterAIController::ExecutePatrolCrawlingBehavior_Implementation(float DeltaTime)
{
	// Default patrol crawling behavior
	// This can be overridden in Blueprint or subclasses to implement actual patrol logic
	// Similar to standing patrol but with different animation/movement style
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
