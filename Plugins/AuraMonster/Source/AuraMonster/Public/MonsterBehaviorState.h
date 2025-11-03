// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MonsterBehaviorState.generated.h"

/**
 * Enum representing the different behavior states for the monster
 */
UENUM(BlueprintType)
enum class EMonsterBehaviorState : uint8
{
	/** Monster is idle, not moving */
	Idle UMETA(DisplayName = "Idle"),
	
	/** Monster is patrolling while standing/walking */
	PatrolStanding UMETA(DisplayName = "Patrol (Standing)"),
	
	/** Monster is patrolling while crawling */
	PatrolCrawling UMETA(DisplayName = "Patrol (Crawling)")
};
