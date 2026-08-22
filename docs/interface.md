# Interface Information

The gripper capabilities package provides one command interface and one state-reading interface.

## GripperCommandRunner

This runner sends a `control_msgs/action/GripperCommand` goal through the active gripper action server.

Input Parameters:

| Parameter Name | Type | Description |
| --- | --- | --- |
| position | double | Target gripper opening for the active driver. |
| max_effort | double | Optional effort limit used during the command. Defaults to `0.0`. |

Output Parameters:

| Parameter Name | Type | Description |
| --- | --- | --- |
| position | double | Final gripper position reported by the action result. |
| effort | double | Final gripper effort reported by the action result. |
| stalled | bool | Whether the gripper stalled before reaching the exact target. |
| reached_goal | bool | Whether the gripper reached the requested target position. |

## GripperStateRunner

This runner reads the current gripper articulation from `/joint_states` and emits vectors directly.

Input Parameters:

| Parameter Name | Type | Description |
| --- | --- | --- |
| joint_names | string[] | Optional ordered filter of gripper joints to return. When omitted, all joints from the latest sample are emitted. |
| timeout_sec | double | Maximum time in seconds to wait for a joint-state message. Defaults to `2.0`. |

Output Parameters:

| Parameter Name | Type | Description |
| --- | --- | --- |
| joint_names | string[] | Ordered gripper joint names returned by the state reader. |
| joint_positions | double[] | Ordered joint positions matching `joint_names`. |
| joint_count | int | Number of joints emitted in the current sample. |