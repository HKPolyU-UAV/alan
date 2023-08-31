/*
    This file is part of ALan - the non-robocentric dynamic landing system for quadrotor

    ALan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ALan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ALan.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * \file aiekf.hpp
 * \date 24/08/2023
 * \author pattylo
 * \copyright (c) AIRO-Lab, RCUAS of Hong Kong Polytechnic University
 * \brief adaptive, iterated, extended Kalman filter for passive pose tracking
 */

#ifndef AIEKF_HPP
#define AIEKF_HPP

#include "tools/essential.h"
#include "cameraModel.hpp"
#include <sophus/se3.hpp>

using namespace std;

namespace kf
{
    #define kfINITIATE 100
    #define kfREINITIATE 101
    #define kfNORMALKF 102

    typedef struct STATE
    {
        Sophus::SE3d X_SE3;
        Sophus::SE3d V_SE3;
        Eigen::MatrixXd PCov;
    } STATE;

    typedef struct MEASUREMENT
    {
        Sophus::SE3d pose_initial_SE3;
        std::vector<Eigen::Vector2d> pts_2d_detected;
        std::vector<Eigen::Vector3d> pts_3d_exists;
    } MEASUREMENT;

    typedef struct FINAL_RETURN
    {
        STATE X;
        Eigen::MatrixXd COV_MAT;

        double residual_error;
        double delta_now_then;
    }FINAL_RETURN;


    class aiekf : public vision::cameraModel
    {
    private:
        /* ================ main flow ================ */
        
        STATE X_current_posterori;       // X @ k
        STATE X_previous_posterori;      // X @ k - 1
        STATE X_current_dynamic_priori;  // X @ k, priori
        STATE X_current_camera_priori;   // X @ k, priori 
        int trackedSize = 0;

        Eigen::MatrixXd F_k;
        Eigen::MatrixXd H_k;

        Eigen::MatrixXd K_k;
        
        Eigen::MatrixXd Q_init;
        Eigen::MatrixXd R_init;
        Eigen::MatrixXd Q_k;
        Eigen::MatrixXd R_k;

        MEASUREMENT Z_current_meas;      // Z @ k       
        

        void setPredict(); //set_X_current_dynamic_priori
        void setMeasurement();
        void doOptimize();
        void setPostOptimize();

        /* ================ utilities ================ */
        double deltaT = 0;
        Eigen::Vector3d g = {0,0,-9.81};
        double QAdaptiveAlpha = 0;
        double RAdaptiveBeta = 0;

        Eigen::VectorXd dfx();
        void setKalmanGain();
        void setAdaptiveQ();
        void setAdaptiveR();
        
        
        void setDFJacobianDynamic(
            Eigen::MatrixXd& Jacob, 
            STATE X_var
        );
        void setDHJacobianCamera(
            Eigen::MatrixXd& Jacob, 
            STATE X_var, 
            Eigen::Vector3d pts_3d
        );

        virtual void setGNBlocks(
            STATE X_var,
            Eigen::MatrixXd& JPJt, 
            Eigen::VectorXd& nJtPf,
            std::vector<Eigen::MatrixXd> Ps,
            std::vector<Eigen::MatrixXd> JacobTemplates
        );

        Eigen::VectorXd dynamicResidual(STATE pose_priori, STATE pose);
        Eigen::Matrix3d skewSymmetricMatrix(const Eigen::Vector3d w);

    public:
        aiekf(){};
        ~aiekf(){
            std::cout<<"EXIT AIEKF"<<std::endl;
        };

        void run_AIEKF(MEASUREMENT meas_at_k, double deltaT_);
        void initKF(
            MEASUREMENT meas_at_k,
            Eigen::MatrixXd Q_init_,
            Eigen::MatrixXd R_init_,
            double QAdaptiveAlpha_,
            double RAdaptiveBeta_
        );
        void reinitKF(MEASUREMENT meas_at_k);

        bool kf_initiated = false; 

    };

}

void kf::aiekf::initKF(
    MEASUREMENT meas_at_k,
    Eigen::MatrixXd Q_init_,
    Eigen::MatrixXd R_init_,
    double QAdaptiveAlpha_,
    double RAdaptiveBeta_
)
{
    X_previous_posterori.X_SE3 = meas_at_k.pose_initial_SE3;
    X_previous_posterori.V_SE3 = Sophus::SE3d(
        Eigen::Matrix3d::Identity(),
        Eigen::Vector3d::Zero()
    );
    trackedSize = 
          X_previous_posterori.X_SE3.log().size()
        + X_previous_posterori.V_SE3.log().head(3).size();

    X_previous_posterori.PCov.resize(trackedSize, trackedSize);
    X_previous_posterori.PCov.setZero();
    X_previous_posterori.PCov.setIdentity();
    X_previous_posterori.PCov = X_previous_posterori.PCov * 2.1;

    if(trackedSize != Q_init_.rows())
    {
        ROS_RED_STREAM("KF DIMENSION DOES NOT MATCH!!!");
        return;
    }
            
    Q_k = this->Q_init = Q_init_;
    R_k = this->R_init = R_init_;

    QAdaptiveAlpha = QAdaptiveAlpha_;
    RAdaptiveBeta  = RAdaptiveBeta_;

    // ros::shutdown();
}

void kf::aiekf::reinitKF(MEASUREMENT meas_at_k)
{
    X_previous_posterori.X_SE3 = meas_at_k.pose_initial_SE3;
    X_previous_posterori.V_SE3 = Sophus::SE3d(
        Eigen::Matrix3d::Identity(),
        Eigen::Vector3d::Zero()
    );

    Q_k = Q_init;
    R_k = R_init;

    // X_previous_posterori.size = X_previous_posterori.size;
}

void kf::aiekf::run_AIEKF(MEASUREMENT meas_at_k, double deltaT_)
{
    Z_current_meas = meas_at_k;
    this->deltaT = deltaT_;

    setPredict();    
    setMeasurement();
    doOptimize();
    // setPostOptimize();
}

void kf::aiekf::setPredict()
{
    Eigen::VectorXd dx = dfx() * deltaT; // first 6 element: se(3) of pose
                                   // last 3 element: velocity

    X_current_dynamic_priori.X_SE3 = 
        X_previous_posterori.X_SE3 * Sophus::SE3d::exp(dx.head(6));

    X_current_dynamic_priori.V_SE3 = 
        X_previous_posterori.V_SE3 * Sophus::SE3d(Eigen::Matrix3d::Identity(), dx.tail(3));

    std::cout<<"just to check frame: "<<std::endl;
    std::cout<<X_current_dynamic_priori.X_SE3.translation()<<std::endl<<std::endl;
    // std::cout<<X_current_dynamic_priori.V_SE3.translation()<<std::endl;

    setDFJacobianDynamic(
        F_k,
        X_previous_posterori
    );

    
    // std::cout<<X_previous_posterori.PCov<<std::endl;

    X_current_dynamic_priori.PCov = 
        F_k * X_previous_posterori.PCov * F_k.transpose() + Q_k;

    // std::cout<<F_k<<std::endl;
    // ros::shutdown();
}

void kf::aiekf::setMeasurement()
{
    X_current_camera_priori.X_SE3 = Z_current_meas.pose_initial_SE3;
    X_current_camera_priori.V_SE3 = Sophus::SE3d::exp(
        Sophus::Vector6d(
            (X_previous_posterori.X_SE3.inverse() * Z_current_meas.pose_initial_SE3).log()
        ) 
        / deltaT
    );

    X_current_camera_priori.PCov = R_k;
}

void kf::aiekf::doOptimize()
{
    std::cout<<R_k.inverse().trace()<<std::endl;
    std::cout<<X_current_dynamic_priori.PCov.inverse().trace()<<std::endl;

    
    const int MAX_ITERATION = 10;
    const double converge_threshold = 1e-6;

    const int points_no = Z_current_meas.pts_2d_detected.size();

    Eigen::MatrixXd JCam;
    JCam.resize(2,9);
    Eigen::MatrixXd JDyn;
    JDyn.resize(9,9);

    Eigen::MatrixXd JPJt; // R9*9
    JPJt.resize(9,9);
    Eigen::VectorXd nJtPf; // R9
    nJtPf.resize(9);

    Eigen::Matrix<double, 9, 1> dx;
    Sophus::Vector6d dx_pose_se3;
    Sophus::Vector6d dx_velo_se3;
    // Eigen::Matrix<double, 6, 1> dx_cam;
    // Eigen::Matrix<double, 6, 1> dx_dyn;

    STATE X_var = X_current_camera_priori;

    int i;
    double cost = 0, lastcost = INFINITY;

    ROS_BLUE_STREAM("OPTIMIZATION METRIC:...");

    std::cout<<"before cam residual:\n "<<get_reprojection_error(
        Z_current_meas.pts_3d_exists,
        Z_current_meas.pts_2d_detected,
        X_var.X_SE3,
        false
    ).norm()<<std::endl<<std::endl;;
    std::cout<<"before dyn residual:\n "
        <<(X_current_dynamic_priori.X_SE3.translation() - X_var.X_SE3.translation()).norm()
        <<std::endl<<std::endl;
    
    std::cout<<"=================================="<<std::endl;
    vector<Eigen::MatrixXd> Ps;
    Ps.emplace_back(R_k);
    Ps.emplace_back(X_current_dynamic_priori.PCov);

    vector<Eigen::MatrixXd> JacobTemplates;
    JacobTemplates.emplace_back(JCam);
    JacobTemplates.emplace_back(JDyn);
    
    for(i = 0; i < MAX_ITERATION; i++)
    {
        setGNBlocks(
            X_var,
            JPJt, 
            nJtPf,
            Ps,
            JacobTemplates
        );

        //solve Adx = b
        dx = JPJt.ldlt().solve(nJtPf);

        // std::cout<<dx<<std::endl<<std::endl;

        for(int i = 0; i < 6; i++)
            if(isnan(dx(i,0)))
                break;

        dx_pose_se3 = dx.head(6);
        dx_velo_se3 << dx.tail(3), Eigen::Vector3d::Zero();

        X_var.X_SE3 =  Sophus::SE3d::exp(dx_pose_se3) * X_var.X_SE3 ;
        X_var.V_SE3 = Sophus::SE3d::exp(dx_velo_se3) * X_var.V_SE3;
        // lastcost = cost;

        if(dx.norm() < converge_threshold)        
            break;
    }

    // std::cout<<Sophus::SE3d(Eigen::Matrix3d::Identity(), Eigen::Vector3d()).matrix()<<std::endl<<std::endl;;
    // std::cout<<Sophus::SE3d(X_var.X_SE3.rotationMatrix(), Eigen::Vector3d(0.2,0.2,0.2)).log()<<std::endl<<std::endl;;

    std::cout<<"\nafter cam residual: "<<get_reprojection_error(
        Z_current_meas.pts_3d_exists,
        Z_current_meas.pts_2d_detected,
        X_var.X_SE3,
        false
    ).norm()<<std::endl;

    // std::cout<<"after"
    std::cout<<"after dyn residual:\n "
        <<(X_current_dynamic_priori.X_SE3.translation() - X_var.X_SE3.translation()).norm()
        <<std::endl<<std::endl;
    // std::cout<<"after dyn residual: "<<dynamicResidual(X_current_dynamic_priori, X_var).norm()<<std::endl;
    std::cout<<"gone thru: "<<i<<" th, end optimize"<<std::endl<<std::endl;;;
    ros::shutdown();
}

void kf::aiekf::setPostOptimize()
{
    // update velocity
    setKalmanGain();
    // X_previous_posterori
    for(int i = 0; i < Z_current_meas.pts_3d_exists.size(); i++)
    {

    }
    // setDHJacobianCamera(H_k, X_current_posterori, Z_current_meas.pts_3d_exists);

    
    // setAdaptiveQ();
    // setAdaptiveR();
}

/* ================ utilities ================ */

Eigen::VectorXd kf::aiekf::dfx()
{
    // x_dot = Ax here  
    // i.e., give x_dot here, basically (in se(3))
    Eigen::VectorXd returnDfx; // first 6 element: se(3) of pose
                               // last 3 element: velocity 
    returnDfx.resize(trackedSize);
    
    X_previous_posterori.V_SE3.log();
    
    returnDfx.head(Sophus::SE3d::DoF) = 
        Sophus::SE3d(
            Eigen::Matrix3d::Identity(),
            X_previous_posterori.V_SE3.translation()
        ).log();
    
    
    returnDfx.tail<Sophus::SE3d::DoF/2>() = X_previous_posterori.X_SE3.rotationMatrix() * g;

    std::cout<<"gravity: "<<std::endl;
    std::cout<<X_previous_posterori.X_SE3.rotationMatrix() * g <<std::endl<<std::endl;;

    return returnDfx;
    
}

void kf::aiekf::setKalmanGain()
{
    K_k = X_current_dynamic_priori.PCov // R 9*9
        * H_k.transpose()                  // R 9*2
        * (H_k * X_current_camera_priori.PCov * H_k.transpose() + R_k).inverse(); // R 2*2
}

void kf::aiekf::setAdaptiveQ()
{
    Eigen::Vector2d innovation;
    innovation = get_reprojection_error(
        Z_current_meas.pts_3d_exists,
        Z_current_meas.pts_2d_detected,
        Z_current_meas.pose_initial_SE3,
        false
    );
    
    Q_k = QAdaptiveAlpha 
            * Q_k 
            + (1 - QAdaptiveAlpha) 
            * (K_k * innovation * innovation.transpose() * K_k.transpose());

}

void kf::aiekf::setAdaptiveR()
{
    Eigen::Vector2d residual;
    residual = get_reprojection_error(
        Z_current_meas.pts_3d_exists,
        Z_current_meas.pts_2d_detected,
        X_current_posterori.X_SE3,
        false
    );

}

void kf::aiekf::setDFJacobianDynamic(
    Eigen::MatrixXd& Jacob,
    STATE X_var
)
{
    Jacob.resize(trackedSize, trackedSize);
    Jacob.setZero();

    Jacob.block<3,3>(0,0).setIdentity();
    Jacob.block<3,3>(3,3).setIdentity();
    Jacob.block<3,3>(6,6).setIdentity();

    Jacob.block<3,3>(0,6).setConstant(deltaT);
    Jacob.block<3,3>(6,3) = 
        -1.0 * 
        skewSymmetricMatrix(X_var.X_SE3.rotationMatrix() * g);
}

void kf::aiekf::setDHJacobianCamera(
    Eigen::MatrixXd& Jacob,
    STATE X_var,
    Eigen::Vector3d pts_3d_exists
)
{
    Eigen::MatrixXd JCamWRTPose;
    JCamWRTPose.resize(2,6);
    Eigen::MatrixXd JCamWRTVelo;
    JCamWRTVelo.resize(2,3);
    
    JCamWRTPose.setZero();
    JCamWRTVelo.setZero();

    solveJacobianCamera(
        JCamWRTPose, 
        X_var.X_SE3, 
        pts_3d_exists
    );
    
    Jacob.resize(JCamWRTPose.rows(), JCamWRTPose.cols() + JCamWRTVelo.cols());
    
    Jacob.block<2,6>(0,0) = JCamWRTPose;
    Jacob.block<2,3>(0,6) = JCamWRTVelo;
}

void kf::aiekf::setGNBlocks(
    STATE X_var,
    Eigen::MatrixXd& JPJt,
    Eigen::VectorXd& nJtPf,
    vector<Eigen::MatrixXd> Ps,
    std::vector<Eigen::MatrixXd> JacobTemplates
)
{
    Eigen::MatrixXd JCamWRTPose;
    JCamWRTPose.resize(2,6);
    Eigen::MatrixXd JDynWRTXNow;
    JDynWRTXNow.resize(9,9);

    Eigen::Matrix<double, 2, 1> eCam; // R2
    Eigen::Matrix<double, 9, 1> eDyn; // R9
    eCam.setZero();
    eDyn.setZero();

    JPJt.setZero();
    nJtPf.setZero();

    // for(auto what : Ps)
    // {
    //     cout<<std::endl<<what.inverse()<<std::endl<<std::endl;
    // }

    for(int i = 0; i < Z_current_meas.pts_3d_exists.size(); i++)
    {
        setDHJacobianCamera(
            JCamWRTPose,
            X_var,
            Z_current_meas.pts_3d_exists[i]
        );

        JCamWRTPose *= (-1);

        eCam = Z_current_meas.pts_2d_detected[i] - reproject_3D_2D(
            Z_current_meas.pts_3d_exists[i],
            X_var.X_SE3
        );

        JPJt += JCamWRTPose.transpose() * Ps[0].inverse() * JCamWRTPose;
        nJtPf += -JCamWRTPose.transpose() * Ps[0].inverse() * eCam;
    }

    for(int i = 0; i < 1; i++)
    {
        setDFJacobianDynamic(
            JDynWRTXNow,
            X_var
        );

        JDynWRTXNow *= (-1);

        eDyn = dynamicResidual(
            X_current_dynamic_priori,
            X_var
        );

        JPJt += JDynWRTXNow.transpose() * Ps[1].inverse() * JDynWRTXNow;
        nJtPf += -JDynWRTXNow.transpose() * Ps[1].inverse() * eDyn;
    }
}


Eigen::VectorXd kf::aiekf::dynamicResidual(
    STATE priori_X,
    STATE X
)
{
    Eigen::VectorXd returnResidual;

    const int poseSize = X.X_SE3.log().size();
    const int velSize  = X.V_SE3.log().head(3).size();
    
    returnResidual.resize(
        X.X_SE3.log().size() + 
        X.V_SE3.log().head<3>().size()
    );

    returnResidual.head(poseSize) = (priori_X.X_SE3 * X.X_SE3.inverse()).log();
    returnResidual.tail(velSize) = (priori_X.V_SE3 * X.V_SE3.inverse()).log().tail(velSize);

    return returnResidual;
}

Eigen::Matrix3d kf::aiekf::skewSymmetricMatrix(const Eigen::Vector3d w)
{
  Eigen::Matrix3d Omega;
  Omega << 0, -w(2), w(1), w(2), 0, -w(0), -w(1), w(0), 0;
  return Omega;
}

#endif