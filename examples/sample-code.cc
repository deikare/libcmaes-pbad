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
#include <thread>
#include <mutex>
#include "simulator/BestSolution.h"

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

void
print_result_to_file(const CMASolutions &cmasols, const std::vector<int> &nodesInLayersCount, std::string filename) {
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << "#include <vector>" << std::endl;

        std::string resultWeights = "std::vector<double> resultWeights = {";
        for (auto x: cmasols.best_candidate().get_x())
            resultWeights += std::to_string(x) + ",";
        if (!resultWeights.empty())
            resultWeights = resultWeights.substr(0, resultWeights.size() - 1);
        resultWeights += "};";
        outputFile << resultWeights << std::endl;

        outputFile << "double feval = " << cmasols.best_candidate().get_fvalue() << ";" << std::endl;

        std::string nodes = "std::vector<int> nodesInLayersCount = {";
        for (auto nodesInLayer: nodesInLayersCount)
            nodes += std::to_string(nodesInLayer) + ",";
        if (!nodes.empty())
            nodes = nodes.substr(0, nodes.size() - 1);
        nodes += "};";
        outputFile << nodes << std::endl;

        double time = double(cmasols.elapsed_time()) / 1000.0;
        outputFile << "double time = " << time << ";" << std::endl;

        outputFile.close();
    } else std::cout << "Couldn't open file " << filename << std::endl;
}

int main() {
    int levelsAmount = 21;
    int firstLayer = 6 + levelsAmount + 1 + Palette::itemTypesNumberLimit +
                     4; //base + levels + 1 for used cps + itemTypes + additional params
    std::vector<int> nodesInLayersCount = {firstLayer, 4, 2};


    nodesInLayersCount.emplace_back(1); //i am giving 1 dimension as output dimension for simplicity in conversion
    int dim = getNumberOfNeuralNetworkWeights(nodesInLayersCount);

    std::cout << "Problem dim: " << dim << std::endl;

//    int dim = 10; // problem dimensions.
    std::vector<double> x0(dim, -0);
//    x0 = resultWeights; // so its starts from ~90% accuracy


    double sigma = 0.1;

    int minLength = 10;
    int maxLength = 50;
    int minCount = 2;
    int maxCount = 50;
    int itemsTypeCount = Palette::itemTypesNumberLimit;
    double difficulty = 1;

    int experimentsPerIteration = 50;

    FitFunc fContainer = [&minLength, &maxLength, &minCount, &maxCount, &itemsTypeCount, &difficulty, &nodesInLayersCount, &levelsAmount, &experimentsPerIteration](
            const double *x, const int N) {

        auto weights = getWeightsMatrix(nodesInLayersCount, x);
        double mean = 0.0;
        std::mutex mtx;
        std::vector<std::thread> threads{};
        for (int i = 0; i < experimentsPerIteration; ++i) {
            std::thread thread(
                    [&minLength, &maxLength, &minCount, &maxCount, &mean, &mtx, &itemsTypeCount, &difficulty, &weights, &levelsAmount]() {
                        Generator generator(minLength, maxLength, minCount, maxCount, itemsTypeCount, difficulty);
                        generator.generate();
                        auto itemTypes = generator.getItems();
                        auto paletteSize = generator.getPaletteSize();

                        Palette palette(paletteSize.first, paletteSize.second, itemTypes, weights, levelsAmount);

                        mtx.lock();
                        mean += palette.performSimulation();
                        mtx.unlock();
                    });
            threads.push_back(std::move(thread));
        }

        for (int i = 0; i < experimentsPerIteration; i++)
            threads[i].join();

        double feval = mean / double(experimentsPerIteration);

        std::cout << "Feval: " << feval << std::endl;
        return feval;
    };
//int lambda = 100; // offsprings at each generation.
    CMAParameters<> cmaparams(x0, sigma);
    cmaparams.set_mt_feval(true);
    cmaparams.set_maximize(true);
    cmaparams.set_ftarget(-0.94); //stupid, because it should be 0.9 in maximize

//todo uncomment for learning
//    CMASolutions cmasols = cmaes<>(fContainer, cmaparams);
//
//    std::string outputFile = "bla3.h";
//    print_result_to_file(cmasols, nodesInLayersCount, outputFile);
//    return cmasols.run_status();


    auto weights = getWeightsMatrix(nodesInLayersCount, &resultWeights[0]);
//    std::list<ItemTypeTuple> items = {{{18, 15}, 11},
//                                      {{19, 16}, 35},
//                                      {{16, 15}, 23},
//                                      {{11, 13}, 49},
//                                      {{18, 19}, 27}};
//    std::list<ItemTypeTuple >items = {{{63, 204}, 20},{{183, 605}, 8},{{71, 168}, 47},{{176, 131}, 4},{{817, 916}, 18},{{342, 371}, 4},{{614, 752}, 39},{{682, 952}, 5},{{194, 511}, 8},{{807, 895}, 36},{{396, 827}, 31},{{271, 871}, 12},{{345, 565}, 14},{{555, 91}, 17},{{726, 485}, 40},{{15, 639}, 26},{{545, 492}, 18},{{695, 401}, 11},{{933, 403}, 23},{{582, 559}, 26}};
//    LengthUnit width = 11459;
//    LengthUnit height = 11459;

    for (int i = 1; i <= itemTypesVector.size(); i++) {
        if (i == 12 || i == 18 || i == 24 || i == 27)
            continue;
        auto items = itemTypesVector[i];
        LengthUnit width = paletteSizes[i].first;
        LengthUnit height = paletteSizes[i].second;

        Palette palette(width, height, items, weights, levelsAmount);
        std::string file = "/home/deikare/wut/pbad-2d-bin-packing/data/data" + std::to_string(i) + ".txt";
        auto result = palette.performSimulationWithSaveToFile(file);
        std::cout << i << ") " << result << std::endl;
    }


}
