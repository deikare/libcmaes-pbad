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
    std::vector<int> nodesInLayersCount = {firstLayer};
    //todo add assert

    nodesInLayersCount.emplace_back(1); //i am giving 1 dimension as output dimension for simplicity in conversion
    int dim = getNumberOfNeuralNetworkWeights(nodesInLayersCount);

    std::cout << "Problem dim: " << dim << std::endl;

//    int dim = 10; // problem dimensions.
    std::vector<double> x0(dim, 150);


    double sigma = 0.1;

    int minLength = 1;
    int maxLength = 50;
    int minCount = 10;
    int maxCount = 20;
    int itemsTypeCount = Palette::itemTypesNumberLimit;
    double difficulty = 1;

    FitFunc fContainer = [minLength, maxLength, minCount, maxCount, itemsTypeCount, difficulty, nodesInLayersCount, levelsAmount](const double *x, const int N) {
        auto weights = getWeightsMatrix(nodesInLayersCount, x);
        Generator generator(minLength, maxLength, minCount, maxCount, itemsTypeCount, difficulty);
        generator.generate();
        auto itemTypes = generator.getItems();
        auto paletteSize = generator.getPaletteSize();

//        std::list<ItemTypeTuple> itemTypes = {
//                {{20,  10},  1},
//                {{10,  10},  1},
//                {{18,  20},  1},
//                {{30,  40},  1},
//                {{30,  25},  1},
//                {{20,  25},  1},
//                {{10,  30},  1},
//                {{13,  30},  1},
//                {{2,   25},  1},
//                {{7,   25},  1},
//                {{40,  90},  1},
//                {{150, 150}, 1},
//                {{20,  10},  1},
//                {{40,  5},   1},
//                {{50,  3},   1},
//                {{25,  8},   1},
//        };

        Palette palette(paletteSize.first, paletteSize.second, itemTypes, weights, levelsAmount);
        auto feval = palette.performSimulation();
        std::cout << "Feval: " << feval * (-1) << std::endl;
        return feval;
    };
    //int lambda = 100; // offsprings at each generation.
    CMAParameters<> cmaparams(x0, sigma);
    cmaparams.set_mt_feval(false);

    //cmaparams._algo = BIPOP_CMAES;
    CMASolutions cmasols = cmaes<>(fContainer, cmaparams);
    std::cout << "best solution: " << cmasols << std::endl;
    std::cout << "optimization took " << cmasols.elapsed_time() / 1000.0 << " seconds\n";
    return cmasols.run_status();
}
