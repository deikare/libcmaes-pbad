/**
 * CMA-ES, Covariance Matrix Evolution Strategy
 * Copyright (c) 2014 INRIA
 * Author: Emmanuel Benazera <emmanuel.benazera@lri.fr>
 *
 * This file is part of libcmaes.
 *
 * libcmaes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libcmaes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcmaes.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmaes.h"
#include <iostream>

using namespace libcmaes;

FitFunc fsphere = [](const double *x, const int N)
{
  double val = 0.0;
  for (int i=0;i<N;i++)
    val += x[i]*x[i];
  return val;
};

// dummy genotype / phenotype transform functions.
TransFunc genof = [](const double *ext, double *in, const int &dim)
{
  for (int i=0;i<dim;i++)
    in[i] = 2.0*ext[i];
};

TransFunc phenof = [](const double *in, double *ext, const int &dim)
{
  for (int i=0;i<dim;i++)
    ext[i] = 0.5*in[i];
};

int main(int argc, char *argv[])
{
  int dim = 10; // problem dimensions.
  //int lambda = 100; // offsprings at each generation.
  GenoPheno<> gp(genof,phenof);
  //CMAParameters cmaparams(dim,lambda);
  CMAParameters<> cmaparams(dim,-1,-1.0,0,gp);
  //cmaparams._algo = BIPOP_CMAES;
  CMASolutions cmasols = cmaes<>(fsphere,cmaparams);
  std::cout << "best solution: " << cmasols << std::endl;
  std::cout << "optimization took " << cmasols._elapsed_time / 1000.0 << " seconds\n";
  return cmasols._run_status;
}