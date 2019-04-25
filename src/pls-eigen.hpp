/**
 * @brief Partial Least Squares Regression with Eigen
 * 
 * Same source as LDA, \cite friedman2001elements
 * 
 * @file pls-eigen.hpp
 * @author François-David Collin <Francois-David.Collin@umontpellier.fr>
 * @brief 
 * @version 0.1
 * @date 2018-11-08
 * 
 * @copyright Copyright (c) 2018
 * 
 */
#pragma once

#include <Eigen/Dense>
#include <range/v3/all.hpp>

using namespace Eigen;
using namespace std;


template<class Derived, class OtherDerived>
VectorXd pls(const MatrixBase<Derived>& x,
         const MatrixBase<OtherDerived>& y,
         size_t ncomp,
         MatrixXd& Projection,
         RowVectorXd& mean,
         RowVectorXd& std)
{
    auto n = x.rows();
    auto p = x.cols();
    mean = x.colwise().mean();
    std = ((x.rowwise() - mean).array().square().colwise().sum() / (x.rows() - 1)).sqrt();;
    MatrixXd X = (x.rowwise() - mean).array().rowwise() / std.array();
    MatrixXd Xo = X;
    MatrixXd XX = X.transpose() * X;
    MatrixXd Y = MatrixXd::Zero(n, ncomp + 1);
    MatrixXd P(p,ncomp);
    MatrixXd R(p,ncomp);
    VectorXd XY(n); 
    VectorXd w(p);
    VectorXd r(p);
    VectorXd t(p);
    // Y.col(0) = y.rowwise() - y.colwise().mean();
    double ymean = y.mean();
    Y.col(0).array() = ymean;
    double SSTO = (y.array() - ymean).array().square().sum();
    for (auto m = 0; m < ncomp; m++)
    {
        XY = X.transpose() * y;
        SelfAdjointEigenSolver<MatrixXd> es( XY.transpose() * XY );
        auto abs_eigenvalues = es.eigenvalues().array().abs();
        size_t max_eigenvalue_indice = ranges::distance(abs_eigenvalues.begin(),ranges::max_element(abs_eigenvalues));
        auto q = es.eigenvectors().col(max_eigenvalue_indice);
        w = XY * q;
        w /= sqrt((w.transpose()*w)(0,0));
        r=w;
        for (auto j=0; j<=m-1;j++)
        {
            r -= (P.col(j).transpose()*w)(0,0)*R.col(j);
        }
        R.col(m) = r;
        t = Xo * r;
        P.col(m) = XX *r/(t.transpose()*t)(0,0);
        VectorXd Zm = X * XY;
        double Znorm = Zm.array().square().sum();
        double Thetam = Zm.dot(y) / Znorm;
        Y.col(m + 1) = Y.col(m) + Thetam * Zm;
        X -= Zm.rowwise().replicate(p) * ((Zm/Znorm).transpose() * X).asDiagonal();
    }
    Projection = R;
    VectorXd res = (Y.block(0,1,n,ncomp).array() - ymean).array().square().colwise().sum() / SSTO;
    return res;
}