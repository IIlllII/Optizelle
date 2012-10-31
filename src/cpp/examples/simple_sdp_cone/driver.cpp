#include <iostream>
#include <iomanip>
#include "peopt/peopt.h"
#include "peopt/vspaces.h"
#include "peopt/json.h"

// Optimize a simple problem with an optimal solution of (0.5,.25)

// Define a simple objective where 
// 
// f(x,y)=-x+y
//
struct MyObj : public peopt::ScalarValuedFunction <double,peopt::Rm> {
    typedef double Real;
    typedef peopt::Rm <Real> X;

    // Evaluation 
    double operator () (const X::Vector& x) const {
        return -x[0]+x[1]; 
    }

    // Gradient
    void grad(
        const X::Vector& x,
        X::Vector& g
    ) const {
        g[0]=Real(-1.);
        g[1]=Real(1.);
    }

    // Hessian-vector product
    void hessvec(
        const X::Vector& x,
        const X::Vector& dx,
        X::Vector& H_dx
    ) const {
        H_dx[0]= Real(0.);
        H_dx[1]= Real(0.);
    }
};

// Define a simple SDP inequality 
//
// g(x,y) = [ y x ] >= 0
//          [ x 1 ]
//
struct MyIneq 
    : public peopt::VectorValuedFunction <double,peopt::Rm,peopt::SQL> 
{
    typedef peopt::Rm <double> X;
    typedef peopt::SQL <double> Y;
    typedef double Real;

    // y=f(x) 
    void operator () (
        const X::Vector& x,
        Y::Vector& y
    ) const {
        y(1,1,1)=x[1];
        y(1,1,2)=x[0];
        y(1,2,1)=x[0];
        y(1,2,2)=Real(1.);
    }

    // y=f'(x)dx
    void p(
        const X::Vector& x,
        const X::Vector& dx,
        Y::Vector& y
    ) const {
        y(1,1,1)=dx[1];
        y(1,1,2)=dx[0];
        y(1,2,1)=dx[0];
        y(1,2,2)=Real(0.);
    }

    // z=f'(x)*dy
    void ps(
        const X::Vector& x,
        const Y::Vector& dy,
        X::Vector& z
    ) const {
        z[0]= Real(2.)*dy(1,1,2);
        z[1]= dy(1,1,1);
    }

    // z=(f''(x)dx)*dy
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
    // Create some type shortcuts
    typedef peopt::Rm <double> X;
    typedef peopt::SQL <double> Z;
    typedef X::Vector X_Vector;
    typedef Z::Vector Z_Vector;

    // Generate an initial guess for the primal
    X_Vector x(2);
    x[0]=1.2; x[1]=3.1;

    // Generate an initial guess for the dual
    std::vector <unsigned int> sizes(1); sizes[0]=2;
    std::vector <peopt::Cone::t> types(1); types[0]=peopt::Cone::Semidefinite;
    Z_Vector z(peopt::Messaging(),types,sizes);
    Z::id(z);

    // Create an optimization state
    peopt::InequalityConstrained <double,peopt::Rm,peopt::SQL>::State::t
        state(x,z);
    
    // Read the parameters from file
    peopt::json::InequalityConstrained <double,peopt::Rm,peopt::SQL>::read(
        peopt::Messaging(),"simple_sdp_cone.peopt",state);

    // Create a bundle of functions
    peopt::InequalityConstrained <double,peopt::Rm,peopt::SQL>::Functions::t
        fns;
    fns.f.reset(new MyObj);
    fns.h.reset(new MyIneq);

    // Solve the optimization problem
    peopt::InequalityConstrained <double,peopt::Rm,peopt::SQL>::Algorithms
        ::getMin(peopt::Messaging(),fns,state);

    // Print out the reason for convergence
    std::cout << "The algorithm converged due to: " <<
        peopt::StoppingCondition::to_string(state.opt_stop) << std::endl;

    // Print out the final answer
    const std::vector <double>& opt_x=*(state.x.begin());
    std::cout << std::setprecision(16) << std::scientific 
        << "The optimal point is: (" << opt_x[0] << ','
	<< opt_x[1] << ')' << std::endl;
}
