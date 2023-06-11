//
// Created by deikare on 28.05.23.
//

#ifndef LIBCMAES_NEURALNETWORK_H
#define LIBCMAES_NEURALNETWORK_H


#include "vector"
#include <cmath>
#include <iostream>
#include <utility>

using Features = std::vector<float>;

class NeuralNetwork {

public:
    explicit NeuralNetwork(std::vector<std::vector<std::vector<float>>> weights) : weights(std::move(weights)) {};

    std::vector<float> simulate(std::vector<float> inputs) {
        int layersCount = getLayersCount();
        std::vector<float> previousLayerNeurons = inputs;


        for (int layerIndex = 1; layerIndex < layersCount; layerIndex++) {
            std::vector<float> layerNeurons;
            int neuronsCount = getLayerSize(layerIndex);

            for (int neuronIndex = 0; neuronIndex < neuronsCount; neuronIndex++) {
                float neuron = calculateNeuron(layerIndex, neuronIndex, previousLayerNeurons);
                layerNeurons.push_back(neuron);
            }

            previousLayerNeurons = layerNeurons;
        }

        return previousLayerNeurons;
    };
private:
    std::vector<std::vector<std::vector<float>>> weights;

    static float calculateSigm(float value) {
        return 1 / (1 + std::exp(-value));
    }

    float calculateNeuron(int layerIndex, int neuronIndex, std::vector<float> previousLayerNeurons) {
        int previousLayerIndex = layerIndex - 1;
        int previousLayerNeuronsCount = previousLayerNeurons.size();
        float value = 0;

        for (int previousLayerNeuronIndex = 0;
             previousLayerNeuronIndex < previousLayerNeuronsCount; previousLayerNeuronIndex++) {
            float weight = weights[previousLayerIndex][previousLayerNeuronIndex][neuronIndex];
            float neuron = previousLayerNeurons[previousLayerNeuronIndex];

            value += weight * neuron;
        }

        // Add bias
        value += weights[previousLayerIndex][previousLayerNeuronsCount][neuronIndex];

        return calculateSigm(value);
    }

    int getLayerSize(int index) {
        if (index == 0) {
            return weights[index].size() - 1;
        }

        return weights[index - 1][0].size();
    }

    int getLayersCount() {
        return weights.size() + 1;
    }

};

#endif //LIBCMAES_NEURALNETWORK_H
