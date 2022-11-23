﻿#include <iostream>
#include "include/essential.h"
#include <decomp_ros_msgs/PolyhedronArray.h>
#include <decomp_geometry/polyhedron.h>

#include "alan_visualization/PolyhedronArray.h"
#include "alan_landing_planning/Traj.h"

#include "visualization_msgs/Marker.h"


#include "./include/rviz_vehicle.hpp"

using namespace std;

static ros::Publisher polyh_vis_pub;
static ros::Publisher traj_vis_pub;

static decomp_ros_msgs::PolyhedronArray sfc_pub_vis_object_polyh;
static decomp_ros_msgs::Polyhedron sfc_pub_vis_object_tangent;
static visualization_msgs::Marker traj_points;





void sfc_msg_callback(const alan_visualization::PolyhedronArray::ConstPtr & msg)
{
    geometry_msgs::Point temp_sfc_p, temp_sfc_n;

    sfc_pub_vis_object_polyh.polyhedrons.clear();   

    // cout<<"size of corridors..."<<msg->a_series_of_Corridor.size()<<endl;
    sfc_pub_vis_object_tangent.points.clear();
    sfc_pub_vis_object_tangent.normals.clear();

    for(auto what : msg->a_series_of_Corridor)
    {
        for(auto whatelse : what.PolyhedronTangentArray)
        {
            // cout<<what.PolyhedronTangentArray.size()<<endl;
            temp_sfc_p.x = whatelse.pt.X;
            temp_sfc_p.y = whatelse.pt.Y;
            temp_sfc_p.z = whatelse.pt.Z;

            temp_sfc_n.x = whatelse.n.X;
            temp_sfc_n.y = whatelse.n.Y;
            temp_sfc_n.z = whatelse.n.Z;

            sfc_pub_vis_object_tangent.points.push_back(temp_sfc_p);
            sfc_pub_vis_object_tangent.normals.push_back(temp_sfc_n);

            sfc_pub_vis_object_polyh.polyhedrons.push_back(sfc_pub_vis_object_tangent);

            // cout<<sf            
        }
        sfc_pub_vis_object_tangent.points.clear();
        sfc_pub_vis_object_tangent.normals.clear();
    }

    sfc_pub_vis_object_polyh.header.frame_id = "map";
}

void traj_msg_callback(const alan_landing_planning::Traj::ConstPtr& msg)
{
    geometry_msgs::Point posi_temp;

    // cout<<msg->trajectory.size()<<endl;

    traj_points.points.clear();
    
    for(auto what : msg->trajectory)
    {
        posi_temp.x = what.position.x;
        posi_temp.y = what.position.y;
        posi_temp.z = what.position.z;
        traj_points.points.push_back(posi_temp);
    }

    

}
static geometry_msgs::PoseStamped uav_pose;
void uavAlanMsgCallback(const alan_landing_planning::AlanPlannerMsg::ConstPtr& msg)
{
    uav_pose.pose.position.x = msg->position.x;
    uav_pose.pose.position.y = msg->position.y;
    uav_pose.pose.position.z = msg->position.z;

    uav_pose.pose.orientation.w = msg->orientation.ow;
    uav_pose.pose.orientation.x = msg->orientation.ox;
    uav_pose.pose.orientation.y = msg->orientation.oy;
    uav_pose.pose.orientation.z = msg->orientation.oz;

}

static geometry_msgs::PoseStamped ugv_pose;
void ugvAlanMsgCallback(const alan_landing_planning::AlanPlannerMsg::ConstPtr& msg)
{
    ugv_pose.pose.position.x = msg->position.x;
    ugv_pose.pose.position.y = msg->position.y;
    ugv_pose.pose.position.z = msg->position.z;

    ugv_pose.pose.orientation.w = msg->orientation.ow;
    ugv_pose.pose.orientation.x = msg->orientation.ox;
    ugv_pose.pose.orientation.y = msg->orientation.oy;
    ugv_pose.pose.orientation.z = msg->orientation.oz;

}

int main(int argc, char** argv)
{    
	ros::init(argc, argv, "gan");
    ros::NodeHandle nh;

    ros::Subscriber sfc_sub = nh.subscribe<alan_visualization::PolyhedronArray>("/alan/sfc/all_corridors", 1, &sfc_msg_callback);
    ros::Subscriber traj_sub = nh.subscribe<alan_landing_planning::Traj>("/alan_visualization/traj", 1, &traj_msg_callback);
    ros::Subscriber uav_pose_sub = nh.subscribe<alan_landing_planning::AlanPlannerMsg>("/AlanPlannerMsg/uav/data", 1, &uavAlanMsgCallback);
    ros::Subscriber ugv_pose_sub = nh.subscribe<alan_landing_planning::AlanPlannerMsg>("/AlanPlannerMsg/ugv/data", 1, &ugvAlanMsgCallback);

    polyh_vis_pub = nh.advertise<decomp_ros_msgs::PolyhedronArray>("/polyhedron_array", 1, true);
    traj_vis_pub = nh.advertise <visualization_msgs::Marker>("/gt_points", 1, true);
    

    traj_points.header.frame_id = "map";
    traj_points.header.stamp = ros::Time::now();
    traj_points.ns = "GT_points";

    traj_points.id = 0;
    traj_points.action = visualization_msgs::Marker::ADD;
    traj_points.pose.orientation.w = 1.0;
    traj_points.type = visualization_msgs::Marker::SPHERE_LIST;
    traj_points.scale.x = traj_points.scale.y = traj_points.scale.z = 0.5;
    std_msgs::ColorRGBA color_for_edge;
    traj_points.color.a=1;
    traj_points.color.g=1;
    traj_points.color.r=0;
    traj_points.color.b=0;

    rviz_vehicle uav_rviz = rviz_vehicle(nh, UAV, false);
    
    Eigen::VectorXd c2b_ugv;
    //x, y, z
    //r, p, y
    c2b_ugv.resize(6);

    c2b_ugv(0);
    c2b_ugv(1);
    c2b_ugv(2);

    c2b_ugv(3);
    c2b_ugv(4);
    c2b_ugv(5);

    rviz_vehicle ugv_rviz = rviz_vehicle(nh, UGV, true, c2b_ugv);

    ros::Rate visrate(20);

    while(ros::ok())
    {

        
        uav_rviz.rviz_pub_vehicle(uav_pose);
        polyh_vis_pub.publish(sfc_pub_vis_object_polyh);
        traj_vis_pub.publish(traj_points);
            
        ros::spinOnce();
        visrate.sleep();

    }

	return 0;
}







    // decomp_ros_msgs::Polyhedron lala;
    // decomp_ros_msgs::PolyhedronArray poly_msg;

    // geometry_msgs::Point pt, n;


    // pt.x = 1;
    // pt.y = 1;
    // pt.z = 1;
    // n.x = 1;
    // n.y = 1;
    // n.z = 0;

    // lala.points.push_back(pt);
    // lala.normals.push_back(n);

    // pt.x = 0;
    // pt.y = 0;
    // pt.z = 1;

    // n.x = -1;
    // n.y = -1;
    // n.z = 0;

    // lala.points.push_back(pt);
    // lala.normals.push_back(n);

    // pt.x = 0;
    // pt.y = 0;
    // pt.z = 4;
    // n.x = 0;
    // n.y = 0;
    // n.z = 1;

    // lala.points.push_back(pt);
    // lala.normals.push_back(n);

    // pt.x = 0;
    // pt.y = 0;
    // pt.z = -1;
    // n.x = 0;
    // n.y = 0;
    // n.z = -1;

    // lala.points.push_back(pt);
    // lala.normals.push_back(n);

    // pt.x = 2;
    // pt.y = -2;
    // pt.z = -1;
    // n.x = 1;
    // n.y = -1;
    // n.z = 0;

    // lala.points.push_back(pt);
    // lala.normals.push_back(n);

    // pt.x = -2;
    // pt.y = 2;
    // pt.z = -1;
    // n.x = -1;
    // n.y = 1;
    // n.z = 0;

    // lala.points.push_back(pt);
    // lala.normals.push_back(n);


    // poly_msg.polyhedrons.push_back(lala);
    // poly_msg.header.frame_id = "map";
    // polyh_vis_pub.publish(poly_msg);