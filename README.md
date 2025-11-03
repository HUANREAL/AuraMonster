# AuraMonster

An Unreal Engine 4 plugin providing a monster AI system with configurable behavior states.

## Overview

AuraMonster is a gameplay plugin that provides a foundation for creating AI-controlled monsters in Unreal Engine 4 projects. The plugin features a state-based behavior system with three core states:

- **Idle** - Monster remains stationary
- **Patrol (Standing)** - Monster patrols while walking upright
- **Patrol (Crawling)** - Monster patrols using custom surface-aware pathfinding that allows crawling on floors, walls, and ceilings

## Features

- Ready-to-use C++ classes for monster characters and AI controllers
- Blueprint-extensible behavior system
- Configurable movement speeds per behavior state
- State transition management with callbacks
- Clean separation between character and AI logic

## Getting Started

See the [Plugin README](Plugins/AuraMonster/README.md) for detailed installation instructions and usage examples.

## Structure

```
Plugins/AuraMonster/
├── AuraMonster.uplugin          # Plugin descriptor
├── README.md                    # Plugin documentation
├── Resources/                   # Plugin resources
└── Source/AuraMonster/          # Source code
    ├── AuraMonster.Build.cs     # Build configuration
    ├── Public/                  # Public headers
    │   ├── AuraMonster.h
    │   ├── MonsterBehaviorState.h
    │   ├── MonsterCharacter.h
    │   └── MonsterAIController.h
    └── Private/                 # Implementation files
        ├── AuraMonster.cpp
        ├── MonsterCharacter.cpp
        └── MonsterAIController.cpp
```

## License

See [LICENSE](LICENSE) for more information.