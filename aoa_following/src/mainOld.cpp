#include <ros/ros.h>

#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Range.h>

#include <nlink_parser/LinktrackAoaNodeframe0.h>
#include <nlink_parser/LinktrackAoaNode0.h>

class MyPubSub{

private:
    ros::NodeHandle nh;
    ros::Publisher pub;
    ros::Subscriber uwbSub;
    ros::Subscriber ultraSub;
    geometry_msgs::Twist vel;
    bool obstacleDetected = false;
    int cnt = 0;


public:
    MyPubSub()
    {
        uwbSub = nh.subscribe("nlink_linktrack_aoa_nodeframe0", 1000, &MyPubSub::uwbCb, this);
        ultraSub = nh.subscribe("ultra", 100, &MyPubSub::ultraCb, this);
        
        pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1000);
    }

    void ultraCb(const sensor_msgs::Range::ConstPtr &msg)
    {
        const float ultra_range = msg->range;
        if (ultra_range <= 20.0){
            cnt++;
            if (cnt == 10)
            {
                //ROS_INFO("Obstacle detected");
                obstacleDetected = true;
                cnt = 0;
            }
        }
        else
            obstacleDetected = false;
    }

    void uwbCb(const nlink_parser::LinktrackAoaNodeframe0::ConstPtr &msg)
    {
        if(!msg->nodes.empty())
        {
            // ROS_INFO("msg LinktrackAoaNodeframe0 received, distance = %.4f", msg->nodes[0].dis);
            // ROS_INFO("msg LinktrackAoaNodeframe0 received, angle = %.4f", msg->nodes[0].angle);

            const float aoa_range = msg->nodes[0].dis;
            const float aoa_angle = msg->nodes[0].angle;

            float linvel, angvel;
            linvel = -0.15;
            angvel = 0.6;


            if(abs(aoa_angle) < 10.0)
            {
                vel.angular.z = 0;

                if(aoa_range < 1.0)
                {
                    vel.linear.x = 0;
                    //ROS_INFO("Near and Alligned");
                }
                else{
                    vel.linear.x = linvel;
                    //ROS_INFO("Alligned");
                }
            }
            else if(abs(aoa_angle) < 70.0){
                if(aoa_angle < 0){
                    if (aoa_range < 1.0)
                    {
                        vel.linear.x = 0;
                        vel.angular.z = -angvel;
                        //ROS_INFO("Static Turn Right");
                    }
                    else{
                        vel.linear.x = linvel;
                        vel.angular.z = -angvel;
                        //ROS_INFO("Turn right");
                    }
                }
                else{
                    if (aoa_range < 1.0)
                    {
                        vel.linear.x = 0;
                        vel.angular.z = angvel;
                        //ROS_INFO("Static Turn Left");
                    }
                    else{
                        vel.linear.x = linvel;
                        vel.angular.z = angvel;
                        //ROS_INFO("Turn left");
                    }
                }
            }
            pub.publish(vel);
        }
    }
};


int main(int argc, char **argv)
{
  ros::init(argc, argv, "uwb_sub_pub");
  MyPubSub my_pub;
  ros::spin();

  return 0;
}
