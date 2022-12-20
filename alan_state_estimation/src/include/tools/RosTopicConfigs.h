/*
    A HPP file for
    ROS subscribe and publish configuration. Use it with SUBPUB_topics.yaml
    Created on 05/12/2022
    (c) pattylo
    from the RCUAS of Hong Kong Polytechnic University
*/

/**
 * \file RosTopicConfigs.hpp
 * \brief classes subscribers and publishers configuration, 
 * \brief just simply pass in ros nodehandle and rosparam namespace 
 */

#ifndef ROSTOPICCONFIGS_HPP
#define ROSTOPICCONFIGS_HPP

#include <ros/ros.h>
#include <iostream>
using namespace std;

#define TOPICTYPE "TOPICTYPE"
#define TOPICNAME "TOPICNAME"
#define SUBORPUB "SUBORPUB"
#define SUB 0
#define PUB 1

#define CAMERA_SUB_TOPIC_A "CAMERA_SUB_TOPIC_A"
#define CAMERA_SUB_TOPIC_B "CAMERA_SUB_TOPIC_B"
#define CAMERA_SUB_TOPIC_C "CAMERA_SUB_TOPIC_C"
#define CAMERA_SUB_TOPIC_D "CAMERA_SUB_TOPIC_D"
#define CAMERA_SUB_TOPIC_E "CAMERA_SUB_TOPIC_E"
#define CAMERA_SUB_TOPIC_F "CAMERA_SUB_TOPIC_F"
#define CAMERA_SUB_TOPIC_G "CAMERA_SUB_TOPIC_G"
#define CAMERA_SUB_TOPIC_H "CAMERA_SUB_TOPIC_H"
#define CAMERA_SUB_TOPIC_I "CAMERA_SUB_TOPIC_I"
#define CAMERA_SUB_TOPIC_J "CAMERA_SUB_TOPIC_J"

#define DEPTH_SUB_TOPIC_A "DEPTH_SUB_TOPIC_A"
#define DEPTH_SUB_TOPIC_B "DEPTH_SUB_TOPIC_B"
#define DEPTH_SUB_TOPIC_C "DEPTH_SUB_TOPIC_C"
#define DEPTH_SUB_TOPIC_D "DEPTH_SUB_TOPIC_D"
#define DEPTH_SUB_TOPIC_E "DEPTH_SUB_TOPIC_E"
#define DEPTH_SUB_TOPIC_F "DEPTH_SUB_TOPIC_F"
#define DEPTH_SUB_TOPIC_G "DEPTH_SUB_TOPIC_G"
#define DEPTH_SUB_TOPIC_H "DEPTH_SUB_TOPIC_H"
#define DEPTH_SUB_TOPIC_I "DEPTH_SUB_TOPIC_I"
#define DEPTH_SUB_TOPIC_J "DEPTH_SUB_TOPIC_J"

#define POSE_SUB_TOPIC_A "POSE_SUB_TOPIC_A"
#define POSE_SUB_TOPIC_B "POSE_SUB_TOPIC_B"
#define POSE_SUB_TOPIC_C "POSE_SUB_TOPIC_C"
#define POSE_SUB_TOPIC_D "POSE_SUB_TOPIC_D"
#define POSE_SUB_TOPIC_E "POSE_SUB_TOPIC_E"
#define POSE_SUB_TOPIC_F "POSE_SUB_TOPIC_F"
#define POSE_SUB_TOPIC_G "POSE_SUB_TOPIC_G"
#define POSE_SUB_TOPIC_H "POSE_SUB_TOPIC_H"
#define POSE_SUB_TOPIC_I "POSE_SUB_TOPIC_I"
#define POSE_SUB_TOPIC_J "POSE_SUB_TOPIC_J"

#define POSE_PUB_TOPIC_A "POSE_PUB_TOPIC_A"
#define POSE_PUB_TOPIC_B "POSE_PUB_TOPIC_B"
#define POSE_PUB_TOPIC_C "POSE_PUB_TOPIC_C"
#define POSE_PUB_TOPIC_D "POSE_PUB_TOPIC_D"
#define POSE_PUB_TOPIC_E "POSE_PUB_TOPIC_E"
#define POSE_PUB_TOPIC_F "POSE_PUB_TOPIC_F"
#define POSE_PUB_TOPIC_G "POSE_PUB_TOPIC_G"
#define POSE_PUB_TOPIC_H "POSE_PUB_TOPIC_H"
#define POSE_PUB_TOPIC_I "POSE_PUB_TOPIC_I"
#define POSE_PUB_TOPIC_J "POSE_PUB_TOPIC_J"

#define IMAGE_PUB_TOPIC_A "IMAGE_PUB_TOPIC_A"
#define IMAGE_PUB_TOPIC_B "IMAGE_PUB_TOPIC_B"
#define IMAGE_PUB_TOPIC_C "IMAGE_PUB_TOPIC_C"
#define IMAGE_PUB_TOPIC_D "IMAGE_PUB_TOPIC_D"
#define IMAGE_PUB_TOPIC_E "IMAGE_PUB_TOPIC_R"
#define IMAGE_PUB_TOPIC_F "IMAGE_PUB_TOPIC_F"
#define IMAGE_PUB_TOPIC_G "IMAGE_PUB_TOPIC_G"
#define IMAGE_PUB_TOPIC_H "IMAGE_PUB_TOPIC_H"
#define IMAGE_PUB_TOPIC_I "IMAGE_PUB_TOPIC_I"
#define IMAGE_PUB_TOPIC_J "IMAGE_PUB_TOPIC_J"

struct CONFIG_SETUP{
    string topictypes;
    string topicnames;
    int SUB_PUB;
};

class RosTopicConfigs
{
private:
    ros::NodeHandle nh;
    XmlRpc::XmlRpcValue TOPICLIST;

    void setupTOPICLIST();
    vector<CONFIG_SETUP> object_ConfigLIST;
    
    
public:
    RosTopicConfigs(ros::NodeHandle& _nh, string param_namespace);
    ~RosTopicConfigs();

    string getTopicName(string designated_name);
};

#endif