#ifndef QPSOLVER_H
#define QPSOLVER_H
#include <OsqpEigen/OsqpEigen.h>

#include "../tools/essential.h"

class osqpsolver
{
private:
    Eigen::VectorXd qpsol;
    vector<Eigen::SparseMatrix<double>> Hessian_array;
    vector<Eigen::SparseMatrix<double>> Alinear_array;
    Eigen::VectorXd _ub;
    Eigen::VectorXd _lb;

    OsqpEigen::Solver qpsolver;

    inline void initiate_qpsolve(int nV, int nC)
    {
        Eigen::VectorXd g;
        g.resize(nV);
        // g << -2, -6;
        g.setZero();

        qpsolver.settings()->setWarmStart(true);
        qpsolver.settings()->setVerbosity(false);

        qpsolver.data()->setNumberOfVariables(nV);
        qpsolver.data()->setNumberOfConstraints(nC);

        if(!qpsolver.data()->setHessianMatrix(Hessian_array[0]))
        cout<<"Hessian not set!"<<endl;

        if(!qpsolver.data()->setGradient(g))
            cout<<"gradient not set!"<<endl;
        
        if(!qpsolver.data()->setLinearConstraintsMatrix(Alinear_array[0]))
            cout<<"linear matrix not set!"<<endl;
        
        if(!qpsolver.data()->setLowerBound(_lb))
            cout<<"lb not set!!"<<endl;
        
        if(!qpsolver.data()->setUpperBound(_ub))
            cout<<"ub not set!"<<endl;
        
        if(!qpsolver.initSolver())
            cout<<"please initialize solver!!"<<endl;

    }

    /* data */
public:
    osqpsolver(/* args */);    
    ~osqpsolver();

    void qp_opt(
        Eigen::MatrixXd _MQM, 
        Eigen::MatrixXd _A, 
        Eigen::MatrixXd _ub, 
        Eigen::MatrixXd _lb);

    void update_b_vectors(Eigen::VectorXd& ub, Eigen::VectorXd& lb)
    {
        _ub = ub;
        _lb = lb;

    }

    void qp_opt_samples();

    inline void set_sampling_matrices(
        vector<Eigen::MatrixXd>& MQM_array,
        vector<Eigen::MatrixXd>& A_array,
        Eigen::VectorXd& ub,
        Eigen::VectorXd& lb
    )
    {
        for(auto& what : MQM_array)
            Hessian_array.emplace_back(what.sparseView());

        for(auto& what : A_array)
            Alinear_array.emplace_back(what.sparseView());
        
        _ub = ub;
        _lb = lb;

        int nV = MQM_array[0].rows();
        int nC = A_array[0].rows();

        cout<<"hi we now in osqpsolver class..."<<endl;
        cout<<Hessian_array.size()<<endl;

        initiate_qpsolve(nV, nC);

        // cout<<_ub<<endl;
        // cout<<_lb<<endl;

        

    };

    inline Eigen::VectorXd getQpsol(){return qpsol;}
    
};

#endif