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

    declare_plan_file_name = DeclareLaunchArgument(
        'filename',
        default_value='navigation_1.xml',
        description='Name of the plan file'
    )

    # plan file folder
    plan_file_path = PathJoinSubstitution([FindPackageShare('nav2_capabilities'), 'plans', plan_file_name ])

    # load config file
    fabric_config = PathJoinSubstitution([FindPackageShare('fabric_server'), 'config', 'fabric.yaml'])

    # launch file path
    fabric_launch_path = os.path.join(get_package_share_directory('fabric_server'), 'launch', 'fabric.launch.py')

    fabric = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(fabric_launch_path),
        launch_arguments={
            'plan_file_path': plan_file_path,
            'fabric_config': fabric_config,
        }.items(),
    )
    
    return LaunchDescription([
        declare_plan_file_name,
        fabric
    ])

