//
// Created by Rachel on 6/20/2019.
//

#include <string>
#include <fstream>
#include "vertex_relabeling/vertex_relabel.hpp"
#include "cpl/cpl_conn.hpp"

const char* prefix = "test";
std::map<int, std::set<int>> bundle_map;

void fetch_data(std::string filename) {
    std::ofstream outfile (filename);

    // Connect to the cpl
    connect_cpl();

    // Get all the bundles in the database
    auto *bundles = new std::vector<cplxx_object_info>();
    get_all_bundles(prefix, bundles);

    std::map<int, std::string> object_types;
    std::map<int, int> new_object_indices;
    int curr_index = 0;

    for (const auto& bundle : *bundles) {
        std::set<int> all_new_object_indices;

        cpl_id_t bundle_id = bundle.id;

        // Get the objects in the bundle
        auto *objects = new std::vector<cplxx_object_info>();
        get_bundle_objects(bundle_id, objects);

        for (const auto& object : *objects) {
            object_types.insert(std::pair<int,std::string>(object.id, std::to_string(object.type)));
        }

        // Get the relations
        auto *relations = new std::vector<cpl_relation_t>();
        get_bundle_relations(bundle_id, relations);

        // Add edge to the data file
        for (const auto& relation : *relations) {
            int source_id = relation.query_object_id;
            int dest_id = relation.other_object_id;

            auto src_type_it = object_types.find(source_id);
            auto dest_type_it = object_types.find(dest_id);

            if (src_type_it != object_types.end() && dest_type_it != object_types.end()) {
                auto src_index_it = new_object_indices.find(source_id);
                int new_src_index = curr_index;
                if (src_index_it != new_object_indices.end()) {
                    new_src_index = src_index_it->second;
                } else {
                    new_object_indices.insert(std::pair<int,int>(source_id, curr_index));
                    curr_index++;
                }

                auto dest_index_it = new_object_indices.find(dest_id);
                int new_dest_index = curr_index;
                if (dest_index_it != new_object_indices.end()) {
                    new_object_indices.insert(std::pair<int,int>(dest_id, curr_index));
                    new_dest_index = dest_index_it->second;
                } else {
                    curr_index++;
                }

                all_new_object_indices.insert(new_src_index);
                all_new_object_indices.insert(new_dest_index);
                std::string relation_str = src_type_it->second + ":" + dest_type_it->second + ":" + std::to_string(relation.type);
                outfile << std::to_string(new_src_index) << "\t" << std::to_string(new_dest_index) << "\t" << relation_str << std::endl;
            } else {
                throw std::runtime_error("Bundle objects and relations are inconsistent");
            }
        }
        bundle_map.insert(std::pair<int,std::set<int>>(bundle.id, all_new_object_indices));
    }
    outfile.close();
}

void parse(type_label &x, const char * s) {
    char * ss = (char *) s;
    char delims[] = ":";
    char * t;
    t = strtok(ss, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Source Type info does not exist" << std::endl;
    assert(t != NULL);
    x.new_src = atoi(t);
    //TODO: We can make sure type value is never 0 so we can check if parse goes wrong here
    t = strtok(NULL, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Destination Type info does not exist" << std::endl;
    assert (t != NULL);
    x.new_dst = atoi(t);
    t = strtok(NULL, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Edge Type info does not exist" << std::endl;
    assert (t != NULL);
    x.edge = atoi(t);
    t = strtok(NULL, delims);
    if (t != NULL)
        logstream(LOG_FATAL) << "Extra info will be ignored" << std::endl;
    return;
}

void countBundleLabels() {
    KernelMaps* km = KernelMaps::get_instance();

    // Collect counts per bundle
    std::map<int, int> label_map = km -> get_label_map();
    std::map<int, std::map<int, int>> all_bundle_stats;

    // Count labels for each bundle separately
    for (auto bundle_it = bundle_map.begin(); bundle_it != bundle_map.end(); bundle_it++ ) {

        // Count of labels for this specific bundle
        std::map<int, int> label_count;
        for (int initial_vertex_label : bundle_it->second) {
            std::cout << initial_vertex_label << "\n";
            // For each original vertex label in the bundle, find what the new label is
            std::map<int,int>::iterator new_vertex_label_it = label_map.find(initial_vertex_label);

            // Increment the counter for this new label
            if (new_vertex_label_it != label_map.end()) {
                std::map<int,int>::iterator it = label_count.find(new_vertex_label_it->second);
                if (it != label_count.end()) {
                    it->second++;
                } else {
                    label_count.insert(std::make_pair(new_vertex_label_it->second, 1));
                }
            } else {
                std::cout << "MISSING: " << std::to_string(initial_vertex_label) << "\n";
                throw std::runtime_error("Label map is missing entries");
            }
        }
        all_bundle_stats.insert(std::pair<int,std::map<int, int>>(bundle_it->first, label_count));
    }

    std::cout << "Printing bundle label counts..." << std::endl;
    for (auto map_itr = all_bundle_stats.begin(); map_itr != all_bundle_stats.end(); map_itr++){
        std::cout << "\nBundle: " << std::to_string(map_itr->first) << "\n";

        for(auto itr2 = map_itr->second.begin(); itr2 != map_itr->second.end(); itr2++) {
            std::cout << std::to_string(itr2->first) << ", " << std::to_string(itr2->second) << "\n";
        }
    }
}

int main(int argc, const char ** argv) {
    // Create the single instance of KernelMap
    KernelMaps* km = KernelMaps::get_instance();
    km->resetMaps();

    graphchi_init(argc, argv);
    metrics m("Detection Framework");
    global_logger().set_log_level(LOG_DEBUG);

    /* Parameters */
    std::string filename    = get_option_string("file"); // Base filename
    int niters              = get_option_int("niters", 4);
    bool scheduler          = false;                    // Non-dynamic version of pagerank.

    fetch_data(filename);

    /* Process input file - if not already preprocessed */
    int nshards             = convert_if_notexists<EdgeDataType>(filename, get_option_string("nshards", "auto"));

    /* Run */
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m);

    VertexRelabel program;
    engine.run(program, niters);

    countBundleLabels();
    metrics_report(m);
    return 0;
}
