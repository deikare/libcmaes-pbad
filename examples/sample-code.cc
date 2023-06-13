/**
 * CMA-ES, Covariance Matrix Adaptation Evolution Strategy
 * Copyright (c) 2014 Inria
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
#include <libcmaes/cmaes.h>
#include <iostream>
#include "simulator/Generator.h"

using namespace libcmaes;


unsigned int getNumberOfNeuralNetworkWeights(const std::vector<int> &nodesInLayersCount) {
    unsigned int result = 0;

    int last = int(nodesInLayersCount.size());

    for (int i = 0; i < last; i++) {
        result += (nodesInLayersCount[i] + 1) * nodesInLayersCount[i + 1];
    }

    return result;
}
//
//
//FitFunc fContainer = [nodesInLayersCount]

std::vector<std::vector<std::vector<float>>>
getWeightsMatrix(const std::vector<int> &neuronsInLayersCount, const double *x) {
    std::vector<std::vector<std::vector<float>>> result;

    int index = 0;

    int size = int(neuronsInLayersCount.size()) - 1;
    for (int layer = 0; layer < size; layer++) {
        int neuronsCountCurrentLayer = neuronsInLayersCount[layer] + 1; // +1 because of bias
        int neuronsCountNextLayer = neuronsInLayersCount[layer + 1];
        result.emplace_back(neuronsCountCurrentLayer, std::vector<float>(neuronsCountNextLayer));
        for (int neuronNumber = 0; neuronNumber < neuronsCountCurrentLayer; neuronNumber++) {
            for (int weightNumber = 0; weightNumber < neuronsCountNextLayer; index++, weightNumber++)
                result[layer][neuronNumber][weightNumber] = x[index];
        }
    }

    return result;
}

int main() {
    int levelsAmount = 21;
    int firstLayer = 6 + levelsAmount + 1 + Palette::itemTypesNumberLimit + 4; //base + levels + 1 for used cps + itemTypes + additional params
//    std::vector<int> nodesInLayersCount = {firstLayer, 4, 2};
    std::vector<int> nodesInLayersCount = {firstLayer, 4, 2};

    //todo add assert

    nodesInLayersCount.emplace_back(1); //i am giving 1 dimension as output dimension for simplicity in conversion
    int dim = getNumberOfNeuralNetworkWeights(nodesInLayersCount);

    std::cout << "Problem dim: " << dim << std::endl;

//    int dim = 10; // problem dimensions.
    std::vector<double> x0(dim, -0);


    double sigma = 0.1;

    int minLength = 10;
    int maxLength = 50;
    int minCount = 2;
    int maxCount = 50;
    int itemsTypeCount = Palette::itemTypesNumberLimit;
    double difficulty = 1;

    int experimentsAmount = 50;

    FitFunc fContainer = [minLength, maxLength, minCount, maxCount, itemsTypeCount, difficulty, nodesInLayersCount, levelsAmount, experimentsAmount](const double *x, const int N) {
        auto weights = getWeightsMatrix(nodesInLayersCount, x);
        double mean = 0.0;
        for (int i = 0; i < experimentsAmount; ++i) {
            Generator generator(minLength, maxLength, minCount, maxCount, itemsTypeCount, difficulty);
            generator.generate();
            auto itemTypes = generator.getItems();
            auto paletteSize = generator.getPaletteSize();

            Palette palette(paletteSize.first, paletteSize.second, itemTypes, weights, levelsAmount);

            mean += palette.performSimulation();
        }
        double feval = mean / double(experimentsAmount);

        std::cout << "Feval: " << feval<< std::endl;
        return feval;
    };
    //int lambda = 100; // offsprings at each generation.
    CMAParameters<> cmaparams(x0, sigma);
    cmaparams.set_mt_feval(true);
    cmaparams.set_maximize(true);
    cmaparams.set_ftarget(-0.9); //stupid, because it should be 0.9 in maximize

    //cmaparams._algo = BIPOP_CMAES;
    CMASolutions cmasols = cmaes<>(fContainer, cmaparams);
    std::cout << "best solution: " << cmasols << std::endl;
    std::cout << "optimization took " << cmasols.elapsed_time() / 1000.0 << " seconds\n";
    return cmasols.run_status();
}
