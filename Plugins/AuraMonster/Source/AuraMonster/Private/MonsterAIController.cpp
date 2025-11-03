// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterAIController.h"
#include "MonsterCharacter.h"
#include "Navigation/PathFollowingComponent.h"

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

	// Initialize timing variables
	CurrentIdleTime = 0.0f;
	TargetIdleDuration = 0.0f;
	TimeSinceLastSubtleMovement = 0.0f;
	NextSubtleMovementTime = 0.0f;
	BreathingCycleTime = 0.0f;
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();

	// Cache reference to the controlled monster
	ControlledMonster = Cast<AMonsterCharacter>(GetPawn());
	
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

	// Validate breathing cycle duration to avoid division by zero
	if (BreathingCycleDuration <= 0.0f)
	{
		BreathingCycleDuration = 4.0f;
	}

	// Update idle time
	CurrentIdleTime += DeltaTime;

	// Update breathing cycle
	BreathingCycleTime += DeltaTime;
	BreathingCycleTime = FMath::Fmod(BreathingCycleTime, BreathingCycleDuration);

	// Calculate breathing intensity (sine wave for smooth breathing)
	const float NormalizedTime = BreathingCycleTime / BreathingCycleDuration;
	float BreathingIntensity = (FMath::Sin(NormalizedTime * 2.0f * UE_PI) + 1.0f) * 0.5f;
	ControlledMonster->OnBreathingUpdate(BreathingIntensity);

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
		float SubtleMin = FMath::Min(MinSubtleMovementInterval, MaxSubtleMovementInterval);
		float SubtleMax = FMath::Max(MinSubtleMovementInterval, MaxSubtleMovementInterval);
		NextSubtleMovementTime = FMath::RandRange(SubtleMin, SubtleMax);
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
			float IdleMin = FMath::Min(MinIdleDuration, MaxIdleDuration);
			float IdleMax = FMath::Max(MinIdleDuration, MaxIdleDuration);
			TargetIdleDuration = FMath::RandRange(IdleMin, IdleMax);
		}
	}
}

void AMonsterAIController::ExecutePatrolStandingBehavior_Implementation(float DeltaTime)
{
	// Default patrol standing behavior
	// This can be overridden in Blueprint or subclasses to implement actual patrol logic
	// For example: move to patrol points, look around, etc.
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
		// Reset idle timing
		CurrentIdleTime = 0.0f;
		
		// Validate idle duration range
		float IdleMin = FMath::Min(MinIdleDuration, MaxIdleDuration);
		float IdleMax = FMath::Max(MinIdleDuration, MaxIdleDuration);
		TargetIdleDuration = FMath::RandRange(IdleMin, IdleMax);
		
		// Reset subtle movement timing
		TimeSinceLastSubtleMovement = 0.0f;
		
		// Validate subtle movement interval range
		float SubtleMin = FMath::Min(MinSubtleMovementInterval, MaxSubtleMovementInterval);
		float SubtleMax = FMath::Max(MinSubtleMovementInterval, MaxSubtleMovementInterval);
		NextSubtleMovementTime = FMath::RandRange(SubtleMin, SubtleMax);
		
		// Reset breathing cycle
		BreathingCycleTime = 0.0f;
	}
}

void AMonsterAIController::OnExitState_Implementation(EMonsterBehaviorState OldState)
{
	// Called when exiting a state
	// Can be overridden to clean up state-specific logic
}
