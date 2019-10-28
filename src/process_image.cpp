#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <numeric>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    
    int white_pos = 0;
    int white_count = 0;

    for (int i = 0; i < img.height; i++)
    {
      for (int j = 0; j < img.width; j++)
      {
        if ((img.data[(i*img.width+j)*3]==white_pixel) && (img.data[(i*img.width+j)*3+1]==white_pixel) && (img.data[(i*img.width+j)*3+2]==white_pixel))
        {
          white_pos += j;
          white_count++;
        }
      }
    }

    if (white_count > 0)
    {
      if (white_pos/white_count < img.width/3)
      {
        // send turn left srv
        ROS_INFO("Turn left - position: %d, count: %d", white_pos/white_count, white_count);
        drive_robot(0.1, 0.2);
      }
      else if (white_pos/white_count < img.width/3*2)
      {
        // send go straight srv
        ROS_INFO("Go straight - position: %d, count: %d", white_pos/white_count, white_count);
        drive_robot(0.125, 0.0);
      }
      else
      {
        // send turn right srv
        ROS_INFO("Turn right - position: %d, count: %d", white_pos/white_count, white_count);
        drive_robot(0.1, -0.2);
      }
    }
    else
    {
      // send stop srv
      ROS_INFO("No white_ball, stop");
      drive_robot(0.0, 0.0);
    }
    
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
