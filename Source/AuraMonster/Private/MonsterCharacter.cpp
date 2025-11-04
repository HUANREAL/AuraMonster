// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterCharacter.h"
#include "MonsterAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
 	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize default state
	CurrentBehaviorState = EMonsterBehaviorState::Idle;

	// Set default movement speeds
	PatrolStandingSpeed = 300.0f;
	PatrolCrawlingSpeed = 150.0f;
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize movement speed based on initial state
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = GetMovementSpeedForState(CurrentBehaviorState);
	}
}

// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMonsterCharacter::SetBehaviorState(EMonsterBehaviorState NewState)
{
	if (CurrentBehaviorState != NewState)
	{
		EMonsterBehaviorState OldState = CurrentBehaviorState;
		CurrentBehaviorState = NewState;

		// Update movement speed based on new state
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = GetMovementSpeedForState(NewState);
		}

		// Notify AI Controller about state change if this was called directly
		// (not from AI Controller's TransitionToState)
		if (AMonsterAIController* AIController = Cast<AMonsterAIController>(GetController()))
		{
			// Only transition if AI Controller is not already in this state
			// This prevents infinite loops when called from AIController->TransitionToState
			if (AIController->GetCurrentState() != NewState)
			{
				AIController->TransitionToState(NewState);
			}
		}

		// Notify about state change
		OnBehaviorStateChanged(OldState, NewState);
	}
}

/**
 * Internal method to set behavior state without triggering AI Controller synchronization.
 * 
 * This method is used exclusively by MonsterAIController during:
 * - Initial state setup in BeginPlay()
 * - State transitions initiated by the AI Controller via TransitionToState()
 * 
 * Unlike SetBehaviorState(), this method does NOT notify the AI Controller of the state change,
 * preventing circular calls. External code should use SetBehaviorState() instead, which provides
 * full bidirectional synchronization between the character and AI controller.
 * 
 * @param NewState The new behavior state to set
 */
void AMonsterCharacter::SetBehaviorStateInternal(EMonsterBehaviorState NewState)
{
	// Internal method used by AI Controller to set state without triggering synchronization
	// This avoids circular calls during initialization and state transitions initiated by AI Controller
	if (CurrentBehaviorState != NewState)
	{
		EMonsterBehaviorState OldState = CurrentBehaviorState;
		CurrentBehaviorState = NewState;

		// Update movement speed based on new state
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = GetMovementSpeedForState(NewState);
		}

		// Notify about state change (but don't sync with AI Controller)
		OnBehaviorStateChanged(OldState, NewState);
	}
}

float AMonsterCharacter::GetMovementSpeedForState(EMonsterBehaviorState State) const
{
	switch (State)
	{
		case EMonsterBehaviorState::Idle:
			return 0.0f;
		
		case EMonsterBehaviorState::PatrolStanding:
			return PatrolStandingSpeed;
		
		case EMonsterBehaviorState::PatrolCrawling:
			return PatrolCrawlingSpeed;
		
		default:
			return 0.0f;
	}
}

void AMonsterCharacter::OnBehaviorStateChanged_Implementation(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState)
{
	// Default implementation - can be overridden in Blueprint or subclasses
}

void AMonsterCharacter::OnNeckTwitch_Implementation()
{
	// Default implementation - can be overridden in Blueprint or subclasses to trigger neck twitch animation
}

void AMonsterCharacter::OnFingerShift_Implementation()
{
	// Default implementation - can be overridden in Blueprint or subclasses to trigger finger shift animation
}

void AMonsterCharacter::OnBreathingUpdate_Implementation(float BreathingIntensity)
{
	// Default implementation - can be overridden in Blueprint or subclasses to update breathing animation
}
