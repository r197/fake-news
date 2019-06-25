//
//  kernelmaps.hpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#ifndef kernel_maps_hpp
#define kernel_maps_hpp

#include <stdio.h>
#include <map>
#include <set>
#include <vector>
#include "logger/logger.hpp"

//We use singleton design pattern
//not thread safe

class KernelMaps {
public:

    static KernelMaps* get_instance();

    ~KernelMaps();

    void resetMaps();

    void insert_label_map();

    int insert_relabel(std::string label);

    void insert_label(int vertex_id, int new_label);

    std::vector<int> generate_count_array(std::map<int, int>& map);

    std::map<int, std::map<int, int>> getLabelCounts(std::map<int, std::set<int>> bundle_map);

    std::map<int, int> get_label_map () {
        return this->label_map;
    }

    std::map<std::string, int> get_relabel_map () {
        return this->relabel_map;
    }

    void print_relabel_map();

    void print_label_map();

private:

    static KernelMaps* single_instance;

    std::map<std::string, int> relabel_map;//a global relabel map

    std::map<int, int> label_map;//a global label map

    int counter;//a counter to facilitate relabeling. This is also the size of the relabel map
};

#endif /* kernel_maps_hpp */
