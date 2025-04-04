# TopDownMovement Plugin for Unreal Engine

<p align="center">
  <img src="images/Demo.gif" alt="Demo" width="600"/>
</p>

## Overview

The **TopDownMovement** plugin is designed for Unreal Engine projects aiming for intuitive, smooth, and flexible top-down camera movement and interactions. It supports multiple input methods including Keyboard/Mouse, Gamepad, and Touch. The plugin provides a solid foundation for creating games with top-down perspectives, such as strategy games, RPGs, or casual puzzle games.

## Features

- **Smooth Camera Movement:** Implemented using `USpringArmComponent` and `UCameraComponent` for seamless camera adjustments.
- **Multiple Input Support:** Automatically detects and switches between Keyboard/Mouse, Gamepad, and Touch inputs.
- **Zoom Functionality:** Dynamically adjustable zoom with interpolations and customizable curves.
- **Edge Scrolling:** Moves the camera when the cursor or touch input approaches the viewport edges.
- **Selectable Actors:** Built-in collision detection to easily handle interactions with game objects.
- **Configurable Input Settings:** Loads input settings directly from configurable `.ini` files.
- **Visual Feedback:** Provides cursor feedback and hover effects for enhanced user interaction.

## Installation

1. **Plugin Integration**

   - Copy the `TopDownMovement` folder to your Unreal Project's `Plugins` directory:
     ```
     YourProject/Plugins/TopDownMovement
     ```

2. **Enable the Plugin**

   - Launch your Unreal Project, navigate to **Edit > Plugins**.
   - Find and enable `TopDownMovement`.
   - Restart Unreal Engine to apply changes.

## Plugin Content Example

<p align="center">
  <img src="images/Content.PNG" alt="Plugin Content Overview" width="700"/>
</p>

This image shows a sample plugin folder structure and assets:
- `BP_Player` and `BP_PlayerController` - Blueprint classes based on the core C++ classes.
- `C_Zoom` - A float curve asset controlling zoom behavior.
- `DA_InputSetup` - A Data Asset for organizing input actions.
- `M_Cursor` - The material for visualizing the cursor.
- `Input/` - Folder containing Input Actions and Mapping Contexts.

## Usage

### Player Pawn Setup

- Use or subclass `ATopDownPlayer` pawn provided by the plugin.
- Adjust properties like `PullStartDistance` and `EdgeMoveDistance` in the Unreal Editor to fine-tune camera behavior.

The plugin uses a timer to consistently update player movement and camera handling:

```cpp
GetWorld()->GetTimerManager().SetTimer(
    MoveTrackingTimerHandle,
    this,
    &ATopDownPlayer::MoveTracking,
    0.01667f,  // Approx. 60 FPS
    true
);
```

This ensures smooth and consistent updates for camera positioning and interaction logic.

### Player Controller Setup

- Use or subclass `ATopDownController` provided by the plugin for automatic input type detection and switching.
- Listen to input type changes through the `OnKeySwitch` delegate to handle custom logic based on input method.

### Extending Functionality

- Functions like `CollisionOverlapCheck`, camera logic (`MoveTracking`), and interaction events (`OnSelection`) can be customized to fit specific game mechanics or interaction models.

For handling custom interactions, call the following Blueprint function:

```cpp
/**
 * Handles the selection logic; implementation depends on the specific project requirements.
 */
UFUNCTION(BlueprintCallable)
void HandleSelection();
```

## Example

To handle input type changes, bind to the controller's delegate in Blueprint or C++:

```cpp
TopDownController->OnKeySwitch.AddDynamic(this, &YourClass::HandleInputSwitch);
```

## Compatibility

- Tested with Unreal Engine 5.5.4

## License

Distributed under the MIT License. See `LICENSE` for more information.


