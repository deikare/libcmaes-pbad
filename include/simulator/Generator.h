//
// Created by deikare on 28.05.23.
//

#ifndef LIBCMAES_GENERATOR_H
#define LIBCMAES_GENERATOR_H


#include <utility>
#include "list"
#include <cstdio>
#include <cstdlib>
#include<ctime>

#include "Palette.h"

class Generator {

public:
    Generator(int min_length, int max_length, int min_count, int max_count, int items_type_count, double difficulty)
            : min_length(min_length), max_length(max_length), min_count(min_count), max_count(max_count),
              items_type_count(items_type_count), difficulty(difficulty) {}

    void generate() {
        srand(time(nullptr));

        std::list<std::pair<ItemType, unsigned long>> new_items;

        for (int i = 0; i < items_type_count; i++) {
            bool isAdded = false;

            int width = (rand() % (max_length - min_length)) + min_length;
            int height = (rand() % (max_length - min_length)) + min_length;
            int count = (rand() % (max_count - min_count)) + min_count;

            std::pair<ItemType, unsigned long> newItem;
            newItem.first.first = width;
            newItem.first.second = height;
            newItem.second = count;

            for (int j = 0; j < new_items.size(); j++) {
                auto list_front = new_items.begin();
                std::advance(list_front, j);

                if (list_front->first.first == newItem.first.first &&
                    list_front->first.second == newItem.first.second) {
                    list_front->second += newItem.second;
                    isAdded = true;
                    break;
                }
            }

            if (!isAdded) {
                new_items.push_back(newItem);
            }
        }

        int paletteArea = 0;

        for (auto item : new_items)
            paletteArea += item.first.first * item.first.second * item.second;
//        for(int i = 0; i < new_items.size(); i++){
//            auto item = new_items.begin();
//
//            paletteArea+= item->first.first * item->first.second * item->second;
//
//            std::advance(item, 1);
//        }

        LengthUnit d = std::ceil(std::sqrt(double(paletteArea) / difficulty));
        palette_size =  std::make_pair(d, d);


        items = new_items;
    }

    std::list<std::pair<ItemType, unsigned long>> getItems() {
        return items;
    }

    std::pair<LengthUnit, LengthUnit> getPaletteSize() {
        return palette_size;
    }

private:
    int min_length;
    int max_length;
    int min_count;
    int max_count;
    int items_type_count;
    double difficulty;

    std::list<std::pair<ItemType, unsigned long>> items;
    std::pair<LengthUnit, LengthUnit> palette_size = std::make_pair(0, 0);
};


#endif //LIBCMAES_GENERATOR_H
