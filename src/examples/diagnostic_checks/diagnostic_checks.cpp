// This example demonstrates how to run a series of diagnostic tests
// on functions and then immediately exit.

#include "optizelle/optizelle.h"
#include "optizelle/vspaces.h"

// Grab Optizelle's Natural type
using Optizelle::Natural;

// Squares its input
template <typename Real>
Real sq(Real x){
    return x*x;
}

// Cubes its input
template <typename Real>
Real cub(Real x){
    return x*x*x;
}

// Quads its input
template <typename Real>
Real quad(Real x){
    return x*x*x*x;
}

// Quints its input
template <typename Real>
Real quint(Real x){
    return x*x*x*x*x;
}

// Define the Rosenbrock function where
//
// f(x,y)=(1-x)^2+100(y-x^2)^2
//
struct Rosenbrock :
    public Optizelle::ScalarValuedFunction <double,Optizelle::Rm>
{
    typedef Optizelle::Rm <double> X;

    // Evaluation of the Rosenbrock function
    double eval(const X::Vector & x) const {
        return sq(1.-x[0])+100.*sq(x[1]-sq(x[0]));
    }

    // Gradient
    void grad(
        X::Vector const & x,
        X::Vector & grad
    ) const {
        grad[0]=-400*x[0]*(x[1]-sq(x[0]))-2*(1-x[0]);
        grad[1]=200*(x[1]-sq(x[0]));
    }

    // Hessian-vector product
    void hessvec(
        X::Vector const & x,
        X::Vector const & dx,
        X::Vector & H_dx
    ) const {
        H_dx[0]= (1200*sq(x[0])-400*x[1]+2)*dx[0]-400*x[0]*dx[1];
        H_dx[1]= -400*x[0]*dx[0] + 200*dx[1];
    }
};

// Define some utility function where
//
// g(x)= [ cos(x1) sin(x2)   ]
//       [ 3 x1^2 x2 + x2 ^3 ]
//       [ log(x1) + 3 x2 ^5 ]
//
struct Utility  : public Optizelle::VectorValuedFunction
    <double,Optizelle::Rm,Optizelle::Rm>
{
    typedef Optizelle::Rm <double> X;
    typedef Optizelle::Rm <double> Y;

    // y=g(x)
    void eval(
        X::Vector const & x,
        Y::Vector & y
    ) const {
        y[0]=cos(x[0])*sin(x[1]);
        y[1]=3.*sq(x[0])*x[1]+cub(x[1]);
        y[2]=log(x[0])+3.*quint(x[1]);
    }

    // y=g'(x)dx
    void p(
        X::Vector const & x,
        X::Vector const & dx,
        Y::Vector & y
    ) const {
        y[0]= -sin(x[0])*sin(x[1])*dx[0]
              +cos(x[0])*cos(x[1])*dx[1];
        y[1]= 6.*x[0]*x[1]*dx[0]
              +(3.*sq(x[0])+3.*sq(x[1]))*dx[1];
        y[2]= 1./x[0]*dx[0]
              +15.*quad(x[1])*dx[1];
    }

    // z=g'(x)*dy
    void ps(
        X::Vector const & x,
        Y::Vector const & dy,
        X::Vector & xhat
    ) const {
        xhat[0]= -sin(x[0])*sin(x[1])*dy[0]
              +6.*x[0]*x[1]*dy[1]
              +1./x[0]*dy[2];
        xhat[1]= cos(x[0])*cos(x[1])*dy[0]
              +(3.*sq(x[0])+3.*sq(x[1]))*dy[1]
              +15.*quad(x[1])*dy[2];
    }

    // z=(g''(x)dx)*dy
    void pps(
        X::Vector const & x,
        X::Vector const & dx,
        Y::Vector const & dy,
        X::Vector & xhat
    ) const {
        xhat[0] = (-cos(x[0])*dx[0]*sin(x[1])-sin(x[0])*cos(x[1])*dx[1])*dy[0]
               +(6.*dx[0]*x[1] + 6.*x[0]*dx[1])*dy[1]
               +(-1./sq(x[0])*dx[0])*dy[2];
        xhat[1] = (-sin(x[0])*dx[0]*cos(x[1])-cos(x[0])*sin(x[1])*dx[1])*dy[0]
               +(6.*x[0]*dx[0]+6.*x[1]*dx[1])*dy[1]
               +(60.*cub(x[1])*dx[1])*dy[2];
    }
};

int main() {

    // Create a type shortcut
    using Optizelle::Rm;

    // Allocate memory for an initial guess and equality multiplier
    auto x = std::vector <double> {1.2, 2.3};
    auto y = std::vector <double> (3);

    // Create an optimization state
    Optizelle::EqualityConstrained <double,Rm,Rm>::State::t state(x,y);

    // Modify the state so that we just run our diagnostics and exit
    state.dscheme = Optizelle::DiagnosticScheme::DiagnosticsOnly;
    state.f_diag = Optizelle::FunctionDiagnostics::SecondOrder;
    state.x_diag = Optizelle::VectorSpaceDiagnostics::Basic;
    state.g_diag = Optizelle::FunctionDiagnostics::SecondOrder;
    state.y_diag = Optizelle::VectorSpaceDiagnostics::EuclideanJordan;
    state.L_diag = Optizelle::FunctionDiagnostics::SecondOrder;

    // Create a bundle of functions
    Optizelle::EqualityConstrained <double,Rm,Rm>::Functions::t fns;
    fns.f.reset(new Rosenbrock);
    fns.g.reset(new Utility);

    // Even though this looks like we're solving an optimization problem,
    // we're actually just going to run our diagnostics and then exit.
    Optizelle::EqualityConstrained <double,Rm,Rm>::Algorithms
        ::getMin(Optizelle::Messaging::stdout,fns,state);
}
