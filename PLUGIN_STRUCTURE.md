# AuraMonster Plugin Structure

```
AuraMonster/                                       # Main plugin directory
├── AuraMonster.uplugin                            # Plugin descriptor file
├── README.md                                      # Comprehensive documentation
├── QUICKSTART.md                                  # Quick start guide with examples
├── IMPLEMENTATION_SUMMARY.md                      # Implementation summary
├── PLUGIN_STRUCTURE.md                            # This file - structure documentation
├── LICENSE                                        # License file
│
├── Resources/                                     # Plugin resources (empty for now)
│
└── Source/
    └── AuraMonster/                               # Module source code
        ├── AuraMonster.Build.cs                   # Build configuration
        │
        ├── Public/                                # Public headers (API)
        │   ├── AuraMonster.h                      # Module interface
        │   ├── MonsterBehaviorState.h             # Behavior state enum
        │   ├── MonsterCharacter.h                 # Monster character class
        │   └── MonsterAIController.h              # AI controller class
        │
        └── Private/                               # Implementation files
            ├── AuraMonster.cpp                    # Module implementation
            ├── MonsterCharacter.cpp               # Character implementation
            └── MonsterAIController.cpp            # AI controller implementation
```

## File Descriptions

### Plugin Files

**AuraMonster.uplugin**
- Plugin descriptor with metadata
- Defines module loading settings
- Specifies UE4 compatibility

**README.md** (Plugin)
- Complete plugin documentation
- Feature descriptions
- Installation instructions
- C++ and Blueprint usage examples
- API reference

**QUICKSTART.md**
- Step-by-step tutorials
- Blueprint implementation examples
- Common patterns and use cases
- Troubleshooting guide

### Source Code

**Build Configuration:**
- `AuraMonster.Build.cs` - Defines module dependencies and build settings

**Module Interface:**
- `AuraMonster.h/cpp` - Module startup and shutdown logic

**Core Classes:**

1. **MonsterBehaviorState.h**
   - `EMonsterBehaviorState` enum
   - Three states: Idle, PatrolStanding, PatrolCrawling
   - Blueprint-exposed

2. **MonsterCharacter.h/cpp**
   - Base character class for monsters
   - State management
   - Movement speed configuration
   - Blueprint-extensible events

3. **MonsterAIController.h/cpp**
   - AI controller for behavior management
   - State machine implementation
   - Per-state behavior execution
   - Blueprint-extensible functions

## Key Features

### State Management
- Three distinct behavior states
- Smooth state transitions
- State change callbacks

### Movement System
- Per-state movement speeds
- Automatic speed adjustment on state change
- Idle state stops movement

### Extensibility
- Blueprint-callable functions
- Blueprint-implementable events
- Easy to subclass in C++ or Blueprint

### Documentation
- Comprehensive README
- Quick start guide
- Usage examples
- API reference

## Usage Flow

```
1. Copy the AuraMonster plugin folder to your project's Plugins/ directory
2. Enable plugin in UE4 Editor
3. Create Blueprint subclass of MonsterCharacter
4. Create Blueprint subclass of MonsterAIController
5. Assign AI controller to monster
6. Implement patrol logic
7. Configure movement speeds
8. Add animations
9. Deploy in level
```

## Class Relationships

```
ACharacter (UE4)
    ↓ inherits
AMonsterCharacter
    ├─ Contains: EMonsterBehaviorState (current state)
    ├─ Contains: Movement speed properties
    └─ Used by: AMonsterAIController

AAIController (UE4)
    ↓ inherits
AMonsterAIController
    ├─ Controls: AMonsterCharacter
    ├─ Manages: State transitions
    └─ Executes: Per-state behaviors
```

## State Machine

```
┌──────────┐
│   Idle   │ ←──────────────────┐
└────┬─────┘                    │
     │                          │
     ├──→ TransitionToState()   │
     │                          │
     ↓                          │
┌─────────────────┐             │
│ PatrolStanding  │ ←───┐       │
└────┬────────────┘     │       │
     │                  │       │
     ├──→ TransitionToState()   │
     │                  │       │
     ↓                  │       │
┌─────────────────┐    │       │
│ PatrolCrawling  │ ───┘       │
└────┬────────────┘            │
     │                         │
     └─────────────────────────┘
         TransitionToState()
```

## Integration Points

### For Game Developers:
1. **Subclass in Blueprint** - Extend MonsterCharacter and MonsterAIController
2. **Implement Behaviors** - Override Execute*Behavior functions
3. **Add Assets** - Meshes, animations, sounds
4. **Configure Properties** - Movement speeds, AI parameters

### For Designers:
1. **Place in Levels** - Drag and drop monster instances
2. **Adjust Parameters** - Tweak speeds and behavior via Details panel
3. **Create Variants** - Different monster types with same base

### For Animators:
1. **Read State** - Get behavior state in Animation Blueprint
2. **Drive State Machine** - Use state to control animations
3. **Blend Transitions** - Smooth animation changes on state switch
