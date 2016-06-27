#include "plugins/gamma_distribution.hpp"
#include "interface/utils.hpp"
#include "interface/random.hpp"
#include "interface/plugin.hpp"

using namespace theta;

/*
Note: the gamma_distribution::sample method is from boost and thus subject to following license.


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


void gamma_distribution::sample(ParValues & result, Random & rnd) const{
    const ParId & pid = *par_ids.begin();
    if(k == 1) {
      while(true){
          //exponential distribution:
          double u = rnd.uniform();
          while(u==0.0) u = rnd.uniform();
          double x = -utils::log(u);
          x*=theta;
          if(x < supp.first || x > supp.second) continue;
          result.set(pid, x);
          return;
      }
    }
    else if(k > 1) {
      while(true) {
        double y = tan(M_PI * rnd.uniform());
        double x = sqrt(2 * k - 1)*y + k - 1;
        if(x <= 0.0) continue;
        if(rnd.uniform() > (1 + y*y) * exp(k-1) * log(x/(k - 1)) - sqrt(2 * k - 1)*y)
          continue;
        x*=theta;
        if(x < supp.first || x > supp.second) continue;
        result.set(pid, x);
        return;
      }
    }
    else { /* k < 1.0 */
      while(true) {
        double u = rnd.uniform();
        // draw y according to exponential distribution:
        double y = rnd.uniform();
        while(y==0.0) y = rnd.uniform();
        y = -utils::log(y);
        double x, q;
        if(u < eoverkp1) {
          x = exp(-y / k);
          q = eoverkp1 * exp(-x);
        } else {
          x = y + 1;
          q = eoverkp1 + (1 - eoverkp1) * pow(x, k - 1);
        }
        if(u >= q)
          continue;
        x*=theta;
        if(x < supp.first || x > supp.second) continue;
        result.set(pid, x);
        return;
      }
    }
}

void gamma_distribution::mode(theta::ParValues & result) const{
    const ParId & pid = *par_ids.begin();
    // this is not really the mode, but the mean, but this is Ok since we want a good 'starting point'
    // for minimization / mcmc and this should do just as well.
    result.set(pid, k * theta);
}


double gamma_distribution::eval_nl(const theta::ParValues & values) const{
    const ParId & pid = *par_ids.begin();
    double value = values.get(pid);
    if(value <= supp.first || value > supp.second) return std::numeric_limits<double>::infinity();
    double result = -(k-1) * utils::log(value) + value / theta;
    return result;
}

const std::pair<double, double> & gamma_distribution::support(const theta::ParId & p) const{
    return supp;
}

double gamma_distribution::width(const theta::ParId & p) const{
    return sqrt(k) * theta;
}

gamma_distribution::gamma_distribution(const theta::Configuration & cfg): supp(0, std::numeric_limits<double>::infinity()){
   par_ids.insert(cfg.pm->get<VarIdManager>()->get_par_id(cfg.setting["parameter"]));
   double mu = cfg.setting["mu"];
   double sigma = cfg.setting["sigma"];
   if(mu <= 0.0) throw ConfigurationException("mean must be > 0");
   if(sigma <= 0.0) throw ConfigurationException("width must be > 0");
   k = (mu * mu) / (sigma * sigma);
   theta = sigma * sigma / mu;
   if(cfg.setting.exists("range")){
       supp.first = std::max<double>(0.0, cfg.setting["range"][0]);
       supp.second = cfg.setting["range"][1].get_double_or_inf();
       if(supp.first < 0.0 || supp.second <= supp.first || k*theta < supp.first || k*theta > supp.second){
           throw ConfigurationException("invalid range given: range must be non-empty, must not contain any non-negative value and must contain the mean (k*theta)");
       }
   }
   eoverkp1 = exp(1) / (k+1);
}


REGISTER_PLUGIN(gamma_distribution)
