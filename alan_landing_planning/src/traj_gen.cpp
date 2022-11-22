

// #include "../tools/essential.h"
// #include "bernstein.hpp"

// #include "osqpsolver.hpp"

#include "include/bezier_lib/traj_gen.h"

namespace alan_traj
{
    traj_gen::traj_gen( 
        bezier_info b_info,  
        bezier_constraints b_constraints, 
        int discrete_freq)
        //for variable set
        :
        _axis_dim(b_info.axis_dim), 
        _n_dim((b_info.n_order + 1) * b_info.m * b_info.axis_dim),

        _n_dim_per_axis((b_info.n_order + 1) * b_info.m),

        //for constraints set
        _start(b_constraints.start), 
        _end(b_constraints.end), 

        _cube_list(b_constraints.cube_list),
        _sfc_list(b_constraints.sfc_list),

        _d_constraints(b_constraints.d_constraints),

        //for cost term
        _n_order(b_info.n_order), _m(b_info.m), _d_order(b_info.d_order), _s(b_info.s),

        //freq
        _discrete_freq(discrete_freq) 
    {
        ROS_WARN("entering traj generator!");        

        // msg_printer("-----------------------------------------------------------------");
        // msg_printer("             ALan v0.1 - Autonomous Landing for UAV");
        // msg_printer("                 (c) Li-yu LO,  Ching-wei Chang");
        // msg_printer("              The Hong Kong Polytechnic University");
        // msg_printer("-----------------------------------------------------------------");

        if(b_constraints.corridor_type == "POLYH")
        {
            //all axis matrices set here

            printf("POLYH constraints...");

            bernstein bezier_base(
                _axis_dim,
                _n_order, _m, _d_order, _s, 
                _start, _end,
                _sfc_list, _d_constraints
            );
            printf("1. variable set: ");
            cout<<_n_dim<<endl;

            //2. constraints set
            //should get everything (all axis) here
            _A = bezier_base.getA(); 
            _ub = bezier_base.getUB();
            _lb = bezier_base.getLB();

            printf("2. constraints set: ");
            // cout<<"_A:"<<endl;
            cout<<_A.rows()<<endl;

            // cout<<"_lb:"<<endl;
            // cout<<_lb<<endl;
            // cout<<"_ub:"<<endl;
            // cout<<_ub<<endl;

            // cout<<"\nsummary:\n";
            // cout<<"A size:\n";
            // cout<<_A.rows()<<endl;
            // cout<<_A.cols()<<endl;
            // cout<<"upper bound size:\n";
            // cout<<_ub.size()<<endl;
            // cout<<"lower bound size:\n";
            // cout<<_lb.size()<<endl;


            
            //3. cost set
            printf("3. cost term: x'M'QMx\n");
            _MQM = bezier_base.getMQM();
            // _MQM = get_nearest_SPD(_MQM);


        }
        else if(b_constraints.corridor_type == "CUBE")
        {
            //all axis matrices set here

            printf("CUBE constraints...");
            bernstein bezier_base(
            _axis_dim,
            _n_order, _m, _d_order, _s,
            _start, _end,
            _cube_list, _d_constraints             
            );//pass everything in one pass
            
            // if
            
            //1. variable set
            // _n_dim = (b_info.n_order + 1) + b_info.m
            printf("1. variable set: ");
            cout<<_n_dim<<endl;

            //2. constraints set
            //should be containing all axises matrices
            _A = bezier_base.getA();
            _ub = bezier_base.getUB();
            _lb = bezier_base.getLB();

            printf("2. constraints set: ");
            // cout<<"_A:"<<endl;
            cout<<_A.rows()<<endl;

            cout<<"_lb:"<<endl;
            cout<<_lb.size()<<endl;
            // cout<<"_ub:"<<endl;
            // cout<<_ub<<endl;

            // cout<<"\nsummary:\n";
            // cout<<"A size:\n";
            // cout<<_A.rows()<<endl;
            // cout<<_A.cols()<<endl;
            // cout<<"upper bound size:\n";
            // cout<<_ub.size()<<endl;
            // cout<<"lower bound size:\n";
            // cout<<_lb.size()<<endl;


            
            //3. cost set
            printf("3. cost term: x'M'QMx\n");
            _MQM = bezier_base.getMQM();

            
            // cout<<_MQM<<endl;
            // _MQM = get_nearest_SPD(_MQM);

        }
        else
        {
            ROS_ERROR("Please check kinematic constraint type...");
        }

    }; 

    void traj_gen::solve_opt(int freq)
    {
        // qpsolver qpsolve(_n_dim, _A.rows());

        // qpsolve.solve_qp(_MQM, _A, _ub, _lb);

        osqpsolver trajSolver;

        trajSolver.qp_opt(_MQM, _A, _ub, _lb);
        PolyCoeff = trajSolver.getQpsol();

        cout<<"size of ctrl pts..."<<PolyCoeff.size()<<endl;

        cout<<"enter set Time Discrete..."<<endl;
        setTimeDiscrete();
        cout<<"enter set Opti Traj..."<<endl;
        setOptiTraj();




        // qpsolve.solve();
        // qpsolve.ifopt_test(_MQM, _A, _ub, _lb);
        // qpsolve.solve_trial();

        
    }


    Eigen::MatrixXd traj_gen::get_nearest_SPD(Eigen::MatrixXd MQM) 
    {
        // make the Hessian to a 
        // symmetric positive semidefinite matrix
        // depends on the solver, whether require this step or not
        
        // From Higham: "The nearest symmetric positive semidefinite matrix in the
        // Frobenius norm to an arbitrary real matrix A is shown to be (B + H)/2,
        // where H is the symmetric polar factor of B=(A + A')/2."


        Eigen::MatrixXd spd;
        Eigen::MatrixXd B = (MQM + MQM.transpose()) / 2;
        
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(B, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::MatrixXd U = svd.matrixU();
        Eigen::MatrixXd V = svd.matrixV();

        Eigen::MatrixXd H = V * svd.singularValues().asDiagonal() * V.transpose();
        spd = (B + H) / 2;
        spd = (spd + spd.transpose()) / 2;


        bool psd_or_not = false;
        double k = 0;

        // cout<<"spd:"<<endl<<spd<<endl;

        while(!psd_or_not && k < 10)
        {
            cout<<k<<endl;
            Eigen::LLT<Eigen::MatrixXd> llt_check(spd);
            if(llt_check.info() == Eigen::NumericalIssue)//
            //try to do cholesky decomposition
            //as if A has A=LL^T
            //A is Hermitian & Positive (semi-)Definite
            {
                // cout<<"Possibly non semi-positive definitie matrix!"<<endl;;
            }   
            else
            {
                psd_or_not = true;
                // cout<<"we got it semi-positive definte!"<<endl;
                continue;
            }

            k = k + 1;

            Eigen::VectorXd spd_eigen_vector = spd.eigenvalues().real();


            double mineig = INFINITY;

            for(int i = 0; i < spd_eigen_vector.size(); i++)
            {
                // cout<<"here's one:"<<endl;
                // cout<<mineig<<endl;
                // cout<<spd_eigen_vector(i)<<endl<<endl;;;

                if(spd_eigen_vector(i) < mineig)
                    mineig = spd_eigen_vector(i);
            }

            Eigen::MatrixXd tweak;
            tweak.setIdentity(spd.rows(), spd.cols());

            double epsd = std::numeric_limits<double>::epsilon();

            double eps_mineig = std::nextafter(mineig, epsd) - mineig;

            tweak = tweak * (-mineig * pow(k,2) + eps_mineig);
            

            spd = spd + tweak;

            
        }

        return spd;
    }

    void traj_gen::msg_printer(char *s)
    {
        // printf("%*s%*s\n",10+strlen(s)/2,s,10-strlen(s)/2," ");
    }

    void traj_gen::setOptiTraj()
    {
        alan_landing_planning::AlanPlannerMsg traj_discrete_pt;
        
        double x_pos = 0, y_pos = 0, z_pos = 0;
        double p_base;

        remove("/home/patty/alan_ws/src/alan/alan_landing_planning/src/test/traj.txt");
        // remove("/home/patty/alan_ws/src/alan/alan_landing_planning/src/test/p_base.txt");

        cout<<"seg_i: "<<_m<<endl;
        cout<<"_n_order: "<<_n_order<<endl;


        cout<<time_vector.size()<<endl;


        for(int seg_i = 0; seg_i < _m; seg_i++)
        {

            cout<<"seg time size: "<<time_vector[seg_i].size()<<endl;

            for(int i = 0; i < time_vector[seg_i].size(); i++)
            {
                x_pos = 0;
                y_pos = 0;
                z_pos = 0;
                
                for(int k = 0; k < _n_order + 1; k++)
                {
                    // cout<<"here: "<<seg_i<<" "<<i<<" "<<k<<endl;
                    // cout<<"here: "<<_n_order<<"  "<<k<<endl;

                    // cout<<nchoosek(_n_order, k)<<endl;

                    

                    p_base = nchoosek(_n_order, k) 
                        * pow(time_vector[seg_i][i], k) 
                        * pow(1 - time_vector[seg_i][i], _n_order - k);
                    // ofstream save("/home/patty/alan_ws/src/alan/alan_landing_planning/src/test/p_base.txt",ios::app);
                    // save<<p_base<<endl;
                
                
                    // save.close();
                    
                    // cout<<"p_Base: "<<p_base<<endl;

                    // cout<< seg_i * (_n_order + 1) + k<<endl;
                    // cout<< seg_i * (_n_order + 1) + k + _n_dim_per_axis<<endl;

                    cout<<PolyCoeff(seg_i * (_n_order + 1) + k)<<endl;
                    cout<<PolyCoeff(seg_i * (_n_order + 1) + k + _n_dim_per_axis)<<endl;
                    
                    x_pos = x_pos + PolyCoeff(seg_i * (_n_order + 1) + k) * p_base;
                    y_pos = y_pos + PolyCoeff(seg_i * (_n_order + 1) + k + _n_dim_per_axis) * p_base;

                    

                    // x_pos(idx) = x_pos(idx) + poly_coef_x((k-1)*(n_order+1)+i+1) * basis_p;
                    // y_pos(idx) = y_pos(idx) + poly_coef_y((k-1)*(n_order+1)+i+1) * basis_p;

                }
                traj_discrete_pt.position.x = x_pos;
                traj_discrete_pt.position.y = y_pos;


                

                ofstream save("/home/patty/alan_ws/src/alan/alan_landing_planning/src/test/traj.txt",ios::app);
                save<<x_pos<<endl;
                save<<y_pos<<endl;
                save<<endl;
                save.close();

                optiTraj.trajectory.emplace_back(traj_discrete_pt);                
            }
        }


        
        cout<<"here are the traj size...: "<<optiTraj.trajectory.size()<<endl;

    }

    void traj_gen::setTimeDiscrete()
    {
        vector<double> time_vector_per_seg;
        double discrete_time_step;
        double seg_time;

        for(auto what : time_vector)
            what.clear();

        for(int i = 0; i < _m; i++)
        {
            seg_time = _s[i];
            discrete_time_step = seg_time * _discrete_freq;

            time_vector_per_seg.clear();

            for(double j = 0; j < discrete_time_step; j++)
            {
                // cout<<1 * j / discrete_time_step + 1 / discrete_time_step<<endl;
                time_vector_per_seg.emplace_back(1 * j / discrete_time_step + 1 / discrete_time_step);
            }            

            time_vector.emplace_back(time_vector_per_seg);            
        }
        
    }

    double traj_gen::nchoosek(int n, int k)
    {
        return pascal[n][k];
    }

}
