// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterAIController.h"
#include "MonsterCharacter.h"
#include "Navigation/PathFollowingComponent.h"

AMonsterAIController::AMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentState = EMonsterBehaviorState::Idle;
	ControlledMonster = nullptr;
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
	// Default idle behavior - do nothing
	// This can be overridden in Blueprint or subclasses
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
	// Called when entering a new state
	// Can be overridden to set up state-specific logic
}

void AMonsterAIController::OnExitState_Implementation(EMonsterBehaviorState OldState)
{
	// Called when exiting a state
	// Can be overridden to clean up state-specific logic
}
