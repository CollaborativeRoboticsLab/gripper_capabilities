# gripper_capabilities

Provides capabilities2 runners for gripper command and state access.

## Supported runners

| Runner | Description |
| --- | --- |
| `GripperCommandRunner` | Sends a `control_msgs/action/GripperCommand` goal with target `position` and optional `max_effort`. |
| `GripperStateRunner` | Reads the current gripper articulation from `/joint_states` and emits `joint_names` plus `joint_positions` vectors. |

For interface details, see [docs/interface.md](./docs/interface.md).

## Example Plans

- [plans/gripper_1.xml](./plans/gripper_1.xml): open, close by position control, then open again.
- [plans/gripper_2.xml](./plans/gripper_2.xml): open, close with explicit `max_effort`, then open again.

The example plans use `position="0.08"` for open and `position="0.0"` for closed. Adjust those values if your active gripper driver uses a different travel range.

## Build

```bash
cd ~/colcon_ws
colcon build --packages-up-to gripper_capabilities --symlink-install
```

## Runtime

Start the active gripper action server, then run one of the gripper plans through the packaged wrapper launch.

```bash
source install/setup.bash
ros2 launch gripper_capabilities system.launch.py filename:=gripper_1.xml
```

The wrapper starts Fabric, Capabilities2, and prompt tools automatically. Add `start_prompt_tools:=false` when you want to exclude `prompt_bridge`, and `start_experience_stack:=true` when you also want Experience and Supervisor in the same launch graph.