#ifndef TRAJ_GEN_H
#define TRAJ_GEN_H

#include "../tools/essential.h"
#include "bernstein.h"
#include "osqpsolver.h"

#include "alan_landing_planning/AlanPlannerMsg.h"

namespace alan_traj
{

    //test gan
    class traj_gen
    {

    private:

        //final result
        Eigen::VectorXd PolyCoeff;
        vector<alan_landing_planning::AlanPlannerMsg> optiTraj;

        int _axis_dim;

        //variable set
        int _n_dim;

        //constraints set
        Eigen::MatrixXd _A;
        Eigen::VectorXd _ub, _lb;

        
        endpt _start, _end;
        vector<corridor> _cube_list;
        dynamic_constraints _d_constraints;
        vector<alan_visualization::Polyhedron> _sfc_list;

        //Cost term
        Eigen::MatrixXd _MQM;
        int _n_order, _m, _d_order;
        vector<double> _s;  

        //math tool
        Eigen::MatrixXd get_nearest_SPD(Eigen::MatrixXd Q);

        //set OptiTraj
        void setOptiTraj();

        //other tool
        void msg_printer(char *s);

        
    public:

        traj_gen( bezier_info b_info,  bezier_constraints b_constraints);
        ~traj_gen(){};

        void solve_opt();

        vector<alan_landing_planning::AlanPlannerMsg> getOptiTraj(){return optiTraj;}


        
    };
}

#endif