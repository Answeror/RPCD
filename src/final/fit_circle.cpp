/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-11
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

/// http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?LU_Matrix_Inversion
// REMEMBER to update "lu.hpp" header includes from boost-CVS
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
namespace
{
    namespace ublas = boost::numeric::ublas;
    /* Matrix inversion routine.
    Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
    template<class T>
    bool InvertMatrix(const ublas::matrix<T>& input, ublas::matrix<T>& inverse) {
        using namespace boost::numeric::ublas;
        typedef permutation_matrix<std::size_t> pmatrix;
        // create a working copy of the input
        matrix<T> A(input);
        // create a permutation matrix for the LU-factorization
        pmatrix pm(A.size1());
        // perform LU-factorization
        int res = lu_factorize(A,pm);
        if( res != 0 ) return false;
        // create identity matrix of "inverse"
        inverse.assign(ublas::identity_matrix<T>(A.size1()));
        // backsubstitute to get the inverse
        lu_substitute(A, pm, inverse);
        return true;
    }
}

#include <cmath>

#include <QDebug>

#include "fit_circle.hpp"

cvcourse::fit_circle::result_type cvcourse::fit_circle::go(const point_container &ps)
{
    //qDebug() << "in go";
    BOOST_ASSERT(ps.size() > 2);

    typedef ublas::matrix<double> mat;
    typedef ublas::vector<double> vec;

    const int n = ps.size();

    mat M(n, 3);
    for (int i = 0; i != n; ++i)
    {
        M(i, 0) = ps[i][0];
        M(i, 1) = ps[i][1];
        M(i, 2) = 1;
    }

    vec y(n);
    for (int i = 0; i != n; ++i)
    {
        y(i) = ps[i][0] * ps[i][0] + ps[i][1] * ps[i][1];
    }

    auto MT = trans(M);
    auto B = prod(MT, M);
    vec c = prod(MT, y);
    
    mat C(B.size1(), B.size2());
    //qDebug() << "b4 inv";
    bool ret = InvertMatrix(mat(B), C);
    //qDebug() << "aft inv" << ret;
    BOOST_ASSERT(ret);

    vec z = prod(C, c);

    result_type result;

    /// calc circle
    {
        result.center[0] = z(0) * 0.5;
        result.center[1] = z(1) * 0.5;
        result.radius = std::sqrt(z(2) + result.center[0] * result.center[0] + result.center[1] * result.center[1]);
    }

    /// calc mse
    {
        result.mse = 0;
        auto sq = [](double x){ return x * x; };
        for (int i = 0; i != n; ++i)
        {
            result.mse += sq(length(ps[i] - result.center) - result.radius);
        }
        result.mse /= n;
    }

    //qDebug() << "---";
    //qDebug() << "center:" << result.center[0] << result.center[1];
    //qDebug() << "radius:" << result.radius;
    //qDebug() << "mse" << result.mse;

    return result;
}

