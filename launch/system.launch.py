import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # get the plan name from launch arguments
    plan_file_name = LaunchConfiguration('filename')
    start_experience_stack = LaunchConfiguration('start_experience_stack')
    start_prompt_tools = LaunchConfiguration('start_prompt_tools')

    declare_plan_file_name = DeclareLaunchArgument(
        'filename',
        default_value='gripper_1.xml',
        description='Name of the plan file'
    )

    declare_start_experience_stack = DeclareLaunchArgument(
        'start_experience_stack',
        default_value='false',
        description='Whether to start the experience and supervisor stack alongside fabric'
    )

    declare_start_prompt_tools = DeclareLaunchArgument(
        'start_prompt_tools',
        default_value='true',
        description='Whether to start prompt_bridge for prompt-based plan generation; set false to disable it'
    )

    # plan file folder
    plan_file_path = PathJoinSubstitution([FindPackageShare('gripper_capabilities'), 'plans', plan_file_name ])

    # load config file
    fabric_config = PathJoinSubstitution([FindPackageShare('fabric_server'), 'config', 'fabric.yaml'])

    # launch file path
    fabric_launch_path = os.path.join(get_package_share_directory('fabric_server'), 'launch', 'fabric.launch.py')

    fabric = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(fabric_launch_path),
        launch_arguments={
            'plan_file_path': plan_file_path,
            'fabric_config': fabric_config,
            'start_experience_stack': start_experience_stack,
            'start_prompt_tools': start_prompt_tools,
        }.items(),
    )
    
    return LaunchDescription([
        declare_plan_file_name,
        declare_start_experience_stack,
        declare_start_prompt_tools,
        fabric
    ])

