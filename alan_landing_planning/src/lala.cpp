#include <iostream>
#include "include/bezier_lib/traj_gen.h"

#include <decomp_ros_msgs/PolyhedronArray.h>
#include <decomp_geometry/polyhedron.h>

#include "alan_visualization/Polyhedron.h"
#include "alan_visualization/Tangent.h"

// #include <decomp_ros_utils/data_ros_utils.h>
// decomp_ros_msg
using namespace std;


int main(int argc, char** argv)
{
    Polyhedron3D polyh_msg;
    // polyh_msg.
    

	  ros::init(argc, argv, "lala");
    ros::NodeHandle nh;

    ros::Publisher poly_pub = nh.advertise<decomp_ros_msgs::PolyhedronArray>("polyhedron_array", 1, true);
    ros::Publisher polyh_pub = nh.advertise<alan_visualization::Polyhedron>("/alan_visualization/polyhedron", 1, true);
    
    alan_visualization::Tangent test;
    // test.n.X

    decomp_ros_msgs::PolyhedronArray poly_msg;
    poly_msg.header.frame_id = "map";
    poly_pub.publish(poly_msg);

	
	  int n_order = 6;
    int m = 1;
    int d_order = 3;

    vector<double> s;

    for(int i = 0; i < m; i++)
        s.push_back(1);
    
    alan_traj::endpt_cond start;
    start.p_ = 50;
    start.v_ = 0;
    start.a_ = 0;
    start.j_ = 0;

    alan_traj::endpt_cond end;
    end.p_ = 60;
    end.v_ = 0;
    end.a_ = 0;
    end.j_ = 0;

    vector<alan_traj::corridor> cube_list;
    alan_traj::corridor cube;

    cube.x_max = 100;
    cube.x_min = 0;//0;
    //
    cube_list.push_back(cube);

    // cube.x_max = 150;
    // cube.x_min = 50;//-OsqpEigen::INFTY;//50;
    // cube_list.push_back(cube);

    // cube.x_max = 230;
    // cube.x_min = 130;//-OsqpEigen::INFTY;//130;
    // cube_list.push_back(cube);
    
    // cube.x_max = 300;
    // cube.x_min = 200;//-OsqpEigen::INFTY;//200;
    // cube_list.push_back(cube);
    
    // cube.x_max = 330;
    // cube.x_min = 230;//-OsqpEigen::INFTY;//230;
    // cube_list.push_back(cube);

    alan_traj::dynamic_constraints d_constraints;
    d_constraints.v_max =  150;
    d_constraints.v_min = -150;//OsqpEigen::INFTY;//-150;
    d_constraints.a_max =  200;
    d_constraints.a_min = -200;//OsqpEigen::INFTY;//-200;
    d_constraints.j_max =  400;
    d_constraints.j_min = -400;//OsqpEigen::INFTY;//-400;

    alan_traj::bezier_info b_info;
    alan_traj::bezier_constraints b_constraints;

    b_info.n_order = n_order;
    b_info.m = m;
    b_info.d_order = d_order;
    b_info.s = s;

    b_constraints.start = start;
    b_constraints.end = end;
    b_constraints.cube_list = cube_list;
    b_constraints.d_constraints = d_constraints;

    double t0 = ros::Time::now().toSec();

	alan_traj::traj_gen traj(b_info, b_constraints);

	traj.solve_opt();

    double t1 = ros::Time::now().toSec();

    cout<<"fps: "<<1/(t1-t0)<<endl;

	return 0;
}