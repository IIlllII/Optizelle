#include "peopt/peopt.h"
#include "peopt/vspaces.h"
#include "peopt/json.h"
#include <iostream>
#include <iomanip>

// Optimize a simple optimization problem with an optimal solution of (1,0)

// Squares its input
template <typename Real>
Real sq(Real x){
    return x*x; 
}

// Define a simple objective where 
// 
// f(x,y)=(1/2)x^2+(1/2)y^2
//
struct MyObj : public peopt::ScalarValuedFunction <double,peopt::Rm> {
    typedef peopt::Rm <double> X;
    typedef double Real;

    // Evaluation 
    double operator () (const X::Vector& x) const {
        return Real(0.5)*sq(x[0])+Real(0.5)*sq(x[1]);
    }

    // Gradient
    void grad(
        const X::Vector& x,
        X::Vector& g
    ) const {
        g[0]=x[0];
        g[1]=x[1];
    }

    // Hessian-vector product
    void hessvec(
        const X::Vector& x,
        const X::Vector& dx,
        X::Vector& H_dx
    ) const {
        H_dx[0]= dx[0]; 
        H_dx[1]= dx[1]; 
    }
};

// Define a simple equality constraint
//
// g(x,y)= [ 2x + y = 2 ] 
//
struct MyEq
    : public peopt::VectorValuedFunction <double,peopt::Rm,peopt::Rm>
{
    typedef peopt::Rm <double> X;
    typedef peopt::Rm <double> Y;
    typedef double Real;

    // y=g(x) 
    void operator () (
        const X::Vector& x,
        Y::Vector& y
    ) const {
        y[0] = Real(2.)*x[0]+x[1]-Real(2.);
    }

    // y=g'(x)dx
    void p(
        const X::Vector& x,
        const X::Vector& dx,
        Y::Vector& y
    ) const {
        y[0] = Real(2.)*dx[0]+dx[1];
    }

    // z=g'(x)*dy
    void ps(
        const X::Vector& x,
        const Y::Vector& dy,
        X::Vector& z
    ) const {
        z[0] = Real(2.)*dy[0];
        z[1] = dy[1];
    }

    // z=(g''(x)dx)*dy
    void pps(
        const X::Vector& x,
        const X::Vector& dx,
        const Y::Vector& dy,
        X::Vector& z
    ) const {
        X::zero(z);
    }
};

int main(){
    // Create a type shortcut
    using peopt::Rm;

    // Generate an initial guess for the primal
    std::vector <double> x(2);
    x[0]=2.1; x[1]=1.1;

    // Generate an initial guess for the dual
    std::vector <double> y(1);
    y[0]=1.; 

    // Create an optimization state
    peopt::EqualityConstrained <double,Rm,Rm>::State::t state(x,y);

    // Read the parameters from file
    peopt::json::EqualityConstrained <double,peopt::Rm,peopt::Rm>::read(
        peopt::Messaging(),"simple_equality.peopt",state);
    
    // Create a bundle of functions
    peopt::EqualityConstrained <double,Rm,Rm>::Functions::t fns;
    fns.f.reset(new MyObj);
    fns.g.reset(new MyEq);

    // Solve the optimization problem
    peopt::EqualityConstrained <double,Rm,Rm>::Algorithms
        ::getMin(peopt::Messaging(),fns,state);

    // Print out the reason for convergence
    std::cout << "The algorithm converged due to: " <<
        peopt::StoppingCondition::to_string(state.opt_stop) << std::endl;

    // Print out the final answer
    const std::vector <double>& opt_x=*(state.x.begin());
    std::cout << std::scientific << std::setprecision(16)
        << "The optimal point is: (" << opt_x[0] << ','
	<< opt_x[1] << ')' << std::endl;
}
