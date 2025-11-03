# Aura Monster Plugin

A comprehensive Unreal Engine 4 plugin for creating AI-controlled monsters with multiple behavior states.

## Features

### Behavior States
The plugin implements three distinct behavior states for monsters:

1. **Idle** - Monster remains stationary and inactive
2. **Patrol (Standing)** - Monster patrols an area while standing/walking upright
3. **Patrol (Crawling)** - Monster patrols an area in a crawling posture

### Core Components

#### EMonsterBehaviorState (Enum)
Defines the available behavior states:
- `Idle` - No movement
- `PatrolStanding` - Upright patrol movement
- `PatrolCrawling` - Crawling patrol movement

#### AMonsterCharacter (Character Class)
Base character class for monsters with:
- Behavior state management
- Configurable movement speeds per state
- Blueprint-callable functions for state queries and transitions
- Event callbacks for state changes

**Key Properties:**
- `PatrolStandingSpeed` (default: 300.0) - Movement speed when patrolling while standing
- `PatrolCrawlingSpeed` (default: 150.0) - Movement speed when patrolling while crawling

**Key Functions:**
- `GetBehaviorState()` - Returns current behavior state
- `SetBehaviorState(EMonsterBehaviorState)` - Changes behavior state
- `GetMovementSpeedForState(EMonsterBehaviorState)` - Returns movement speed for a given state
- `OnBehaviorStateChanged(OldState, NewState)` - Event called when state changes (Blueprint implementable)

#### AMonsterAIController (AI Controller)
AI controller that manages monster behavior:
- State machine implementation
- Per-state behavior execution
- State transition management
- Blueprint-extensible behavior functions

**Key Functions:**
- `TransitionToState(EMonsterBehaviorState)` - Transition to a new state
- `GetCurrentState()` - Returns current state
- `ExecuteIdleBehavior(DeltaTime)` - Override to implement idle behavior
- `ExecutePatrolStandingBehavior(DeltaTime)` - Override to implement standing patrol behavior
- `ExecutePatrolCrawlingBehavior(DeltaTime)` - Override to implement crawling patrol behavior
- `OnEnterState(NewState)` - Event called when entering a state
- `OnExitState(OldState)` - Event called when exiting a state

## Installation

1. Copy the `Plugins/AuraMonster` folder to your Unreal Engine 4 project's `Plugins` directory
2. If `Plugins` directory doesn't exist, create it in your project root
3. Regenerate project files (right-click .uproject → Generate Visual Studio project files)
4. Open your project in Unreal Engine 4
5. Enable the plugin: Edit → Plugins → Search for "Aura Monster" → Check the box → Restart Editor

## Usage

### In C++

1. **Create a Monster Character:**
```cpp
#include "MonsterCharacter.h"

// In your code
AMonsterCharacter* Monster = GetWorld()->SpawnActor<AMonsterCharacter>(
    AMonsterCharacter::StaticClass(),
    SpawnLocation,
    SpawnRotation
);
```

2. **Set Behavior State:**
```cpp
Monster->SetBehaviorState(EMonsterBehaviorState::PatrolStanding);
```

3. **Create Custom AI Controller:**
```cpp
#include "MonsterAIController.h"

class AMyMonsterAI : public AMonsterAIController
{
    virtual void ExecutePatrolStandingBehavior_Implementation(float DeltaTime) override
    {
        // Implement your patrol logic here
        // e.g., move to waypoints, detect enemies, etc.
    }
};
```

### In Blueprint

1. **Create Monster Blueprint:**
   - Create a new Blueprint class based on `MonsterCharacter`
   - Customize appearance, animations, and properties
   - Set `PatrolStandingSpeed` and `PatrolCrawlingSpeed` as desired

2. **Create AI Controller Blueprint:**
   - Create a new Blueprint class based on `MonsterAIController`
   - Override behavior functions:
     - `Execute Idle Behavior`
     - `Execute Patrol Standing Behavior`
     - `Execute Patrol Crawling Behavior`
   - Implement your AI logic (waypoint navigation, detection, etc.)

3. **Assign AI Controller:**
   - In your Monster Blueprint, set "AI Controller Class" to your custom AI controller
   - Or set "Auto Possess AI" to "PlacedInWorldOrSpawned"

4. **Change States in Blueprint:**
   ```
   Call "Transition To State" on AI Controller
   Or "Set Behavior State" on Monster Character
   ```

### Example Behavior Implementation

Here's an example of implementing a simple patrol behavior:

```cpp
void AMyMonsterAI::ExecutePatrolStandingBehavior_Implementation(float DeltaTime)
{
    if (!ControlledMonster) return;
    
    // Example: Move to random location periodically
    if (!GetPathFollowingComponent()->HasReached())
    {
        // Still moving to current target
        return;
    }
    
    // Generate random patrol point
    FVector CurrentLocation = ControlledMonster->GetActorLocation();
    FVector RandomOffset = FMath::VRand() * 1000.0f; // 1000 units radius
    RandomOffset.Z = 0; // Keep on ground level
    FVector TargetLocation = CurrentLocation + RandomOffset;
    
    // Move to target
    MoveToLocation(TargetLocation);
}
```

## Extending the Plugin

### Adding New States

To add new behavior states:

1. Add new enum value to `EMonsterBehaviorState` in `MonsterBehaviorState.h`
2. Add case in `AMonsterCharacter::GetMovementSpeedForState()` 
3. Add corresponding property for movement speed
4. Add execute function in `AMonsterAIController`
5. Add case in `AMonsterAIController::Tick()`

### Custom Events

Override these events in Blueprint or C++:
- `OnBehaviorStateChanged` - React to state changes in Monster Character
- `OnEnterState` / `OnExitState` - React to state transitions in AI Controller

## Requirements

- Unreal Engine 4.20 or later
- C++ development tools (Visual Studio)

## License

See LICENSE file in the repository root.
