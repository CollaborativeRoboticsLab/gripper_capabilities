# Interface Information

Following are the details of the interfaces provided by the Nav2 Capabilities package. A runner that intends to connect with one of these interfaces should support the input and ouput parameters described below.

## RobotPoseRunner

Input Parameters:

| Parameter Name    | Type   | Description |
| ---               | ---    | ---         |
| map               | string | Frame id of the map |
| odom              | string | Frame id of the odometry |
| robot             | string | Frame id of the robot |
| time_buffer       | double | TF lookup timeout in seconds before the runner reports failure |

Output Parameters:

| Parameter Name   | Type   | Description |
| ---              | ---    | ---         |
| x                | double | The x coordinate of the robot's pose |
| y                | double | The y coordinate of the robot's pose |
| z                | double | The z coordinate of the robot's pose |
| qx               | double | The x coordinate of the robot's orientation in quaternion |
| qy               | double | The y coordinate of the robot's orientation in quaternion |
| qz               | double | The z coordinate of the robot's orientation in quaternion |
| qw               | double | The w coordinate of the robot's orientation in quaternion |

## WaypointRunner

Input Parameters:

| Parameter Name   | Type   | Description |
| ---              | ---    | ---         |
| x                | double | The x coordinate of the robot's goal |
| y                | double | The y coordinate of the robot's goal |
| z                | double | The z coordinate of the robot's goal |
| qx               | double | The x coordinate of the robot's goal orientation in quaternion |
| qy               | double | The y coordinate of the robot's goal orientation in quaternion |
| qz               | double | The z coordinate of the robot's goal orientation in quaternion |
| qw               | double | The w coordinate of the robot's goal orientation in quaternion |

Output Parameters:

None