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
    auto result = label_map.insert(std::pair<int, int>(vertex_id, new_label));
    if (!result.second) {
        label_map[vertex_id] = new_label;
    }
}

std::vector<int> KernelMaps::generate_count_array(std::map<int, int>& map) {
    std::vector<int> rtn;
    std::map<int, int>::iterator itr = map.begin();
    for (int i = 0; i < this->counter; i++) {
        if (itr != map.end()) {
            if (itr->first == i) {
                rtn.push_back(itr->second);
                itr++;
            } else rtn.push_back(0);
        } else {
            rtn.push_back(0);
        }
    }
    return rtn;
}

std::map<int, std::map<int, int>> KernelMaps::getLabelCounts(std::map<int, std::set<int>> bundle_map) {
    KernelMaps* km = KernelMaps::get_instance();

    // Collect counts per bundle
    std::map<int, int> label_map = km -> get_label_map();
    std::map<int, std::map<int, int>> all_bundle_stats;

    // Count labels for each bundle separately
    for (auto bundle_it = bundle_map.begin(); bundle_it != bundle_map.end(); bundle_it++ ) {

        // Count of labels for this specific bundle
        std::map<int, int> label_count;
        for (int initial_vertex_label : bundle_it->second) {

            // For each original vertex label in the bundle, find what the new label is
            std::map<int,int>::iterator new_vertex_label_it = label_map.find(initial_vertex_label);

            // Increment the counter for this new label
            if (new_vertex_label_it != label_map.end()) {
                auto result = label_count.insert(std::make_pair(new_vertex_label_it->second, 1));
                std::map<int,int>::iterator it = label_count.find(new_vertex_label_it->second);
                if (!result.second) {
                    result.first->second++;
                }
            } else {
                throw std::runtime_error("Label map is missing entries");
            }
        }
        all_bundle_stats.insert(std::pair<int,std::map<int, int>>(bundle_it->first, label_count));
    }

    // Print for debugging
//    std::cout << "Printing bundle label counts..." << std::endl;
//    for (auto map_itr = all_bundle_stats.begin(); map_itr != all_bundle_stats.end(); map_itr++){
//        std::cout << "\nBundle: " << std::to_string(map_itr->first) << "\n";
//
//        for(auto itr2 = map_itr->second.begin(); itr2 != map_itr->second.end(); itr2++) {
//            std::cout << std::to_string(itr2->first) << ", " << std::to_string(itr2->second) << "\n";
//        }
//    }

    return all_bundle_stats;
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

