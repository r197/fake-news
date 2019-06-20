//
//  kernelmaps.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#include "kernel_maps.hpp"

KernelMaps* KernelMaps::single_instance;

KernelMaps* KernelMaps::get_instance() {
    if (!single_instance)
        single_instance = new KernelMaps();
    return single_instance;
}

KernelMaps::~KernelMaps() {
    this->relabel_map.clear();
    this->label_map.clear();
    this->counter = 0;
    delete single_instance;
}

void KernelMaps::resetMaps() {
    this->relabel_map.clear();
    this->label_map.clear();
    this ->counter = 0;
}


//insert a label into the relabel map if it does not exist in the relabel map and then return the new relabeled label
//if it does exist, return the existing relabeled label
int KernelMaps::insert_relabel(std::string label) {
    std::pair<std::map<std::string, int>::iterator, bool> rst;
    rst = relabel_map.insert(std::pair<std::string, int>(label, counter));
    if (rst.second == false) {
        logstream(LOG_INFO) << "Label " + label + " is already in the map." << std::endl;
        return rst.first->second;
    } else {
        counter++;
        return counter - 1;
    }
}

void KernelMaps::insert_label(int vertex_id, int new_label) {
    auto vertex_id_it = label_map.find(vertex_id);
    if (vertex_id_it != label_map.end()) {
        label_map[vertex_id] = new_label;
    } else {
        label_map.insert(std::pair<int, int>(vertex_id, new_label));
    }
    return;
}

//The rest functions are for debugging purpose only
void KernelMaps::print_relabel_map () {
    std::map<std::string, int>::iterator map_itr;
    logstream(LOG_INFO) << "Printing relabel map..." << std::endl;
    for (map_itr = this->relabel_map.begin(); map_itr != this->relabel_map.end(); map_itr++)
        logstream(LOG_INFO) << map_itr->first << ":" << map_itr->second << std::endl;
}

void KernelMaps::print_label_map () {
    std::map<int, int>::iterator map_itr;
    logstream(LOG_INFO) << "Printing label map..." << std::endl;
    for (map_itr = this->label_map.begin(); map_itr != this->label_map.end(); map_itr++)
        logstream(LOG_INFO) << map_itr->first << ":" << map_itr->second << std::endl;
}

