#define BOOST_TEST_MODULE KolmogorovSmirnoffTest
#include <boost/test/unit_test.hpp>

#include "ks.hpp"

std::vector<double> x = { 0.27781608, 0.44903831, -1.06795827, -1.49343677, 1.14217946, -0.90258175,
0.49311432, 1.85239121, 1.27502281, -0.40252451, -0.49373568, -0.19985152,
-2.08639076, 0.83324533, 1.64266160, 0.59820917, 0.88644006, -1.25721474,
0.61210158, -0.15402684, -0.16029270, 2.05644381, 0.69748632, 0.33016235,
0.33497213, 0.07080324, -0.11481628, -0.68166453, 0.60806603, 0.35755078,
1.84785474, -1.72121171, 0.31318918, 1.73366989, 1.00233694, 0.35755547,
0.28978570, -0.37795334, -0.65936246, 0.15246095, 1.50726840, 1.29124919,
1.04480445, 0.17787545, -0.44492940, -0.31645279, -1.63034354, -1.17279017,
-0.02114708, -0.20416522};

std::vector<double> y = { 1.891418850, -1.205411675, -0.297229152, 0.706283885, -1.076878021,
-0.345350583, -0.432461617, -0.328168869, 0.110594500, -0.918126595,
0.286836754, -1.501973089, 0.657761152, -0.117140885, -0.755229058,
-0.155983374, -2.194050191, -2.138459073, 1.040694202, -2.163971814,
-1.394041172, -0.334161508, -0.269530739, -1.039387033, 1.692021929,
0.630056804, -1.062912214, -2.712134959, 0.202180532, -1.306079336,
-1.521460034, 1.609911814, -0.473748672, -2.194428053, -0.751678872,
0.190552565, -0.796884840, -0.887598678, 0.820903464, -0.133406997,
0.278550111, 0.008077645, 0.745780906, 0.417137468, 0.326869921,
0.372459953, 0.925945152, -0.750986581, 0.327844328, -1.236515677 };


BOOST_AUTO_TEST_CASE(UniformKS, *boost::unit_test::tolerance(1e-10))
{
    auto D = KSTest(x,y);
    auto pvalue = 1-psmirnov2x(D,x.size(),y.size());
    BOOST_TEST(D == 0.24);
    BOOST_TEST(pvalue == 0.11238524845512376515);
}
