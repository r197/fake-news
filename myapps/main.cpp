//
//  main.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#include <string>
#include <fstream>
#include "vertex_relabeling/vertex_relabel.hpp"
#include "cpl/cpl_conn.hpp"
#include "profile/profile.hpp"

#define KULLBACKLEIBLER 0
#define HELLINGER 1
#define EUCLIDEAN 2

const char* prefix = "test";

// Get all the objects for the cpl
// TODO: modify so that user can specify what bundles they want (instead of all of them)
std::map<int, std::set<int>> fetch_data(std::string filename) {
    std::ofstream outfile (filename);
    std::map<int, std::set<int>> bundle_map;

    // Connect to the cpl
    connect_cpl();

    // Get all the bundles in the database
    auto *bundles = new std::vector<cplxx_object_info>();
    get_all_bundles(prefix, bundles);

    std::map<int, std::string> object_types; // Map of original object ID to object type
    std::map<int, int> new_object_indices; //Map of original object ID to new object ID
    int curr_index = 0;

    for (const auto& bundle : *bundles) {
        std::set<int> all_new_object_indices; // Keeps track of which objects are in this bundle

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

            // Find the types of the source and dest objects
            auto src_type_it = object_types.find(source_id);
            auto dest_type_it = object_types.find(dest_id);

            if (src_type_it != object_types.end() && dest_type_it != object_types.end()) {

                // Find the new id for the source and dest objects
                auto src_elem = new_object_indices.insert(std::pair<int,int>(source_id, curr_index));
                if (src_elem.second) {
                    curr_index++;
                }
                int new_src_index = src_elem.first->second;

                auto dest_elem = new_object_indices.insert(std::pair<int,int>(dest_id, curr_index));
                if (dest_elem.second) {
                    curr_index++;
                }
                int new_dest_index = dest_elem.first->second;

                // Add objects to bundle objects set, and write edge to file
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
    return bundle_map;
}

// Parser used by graphchi preprocessing (taken from frappuccino)
void parse(type_label &x, const char * s) {
    char * ss = (char *) s;
    char delims[] = ":";
    char * t;
    t = strtok(ss, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Source Type info does not exist" << std::endl;
    assert(t != NULL);
    x.new_src = atoi(t);
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

int main(int argc, const char ** argv) {
    // Create the single instance of KernelMap
    KernelMaps* km = KernelMaps::get_instance();
    km->resetMaps();

    graphchi_init(argc, argv);
    metrics m("Detection Framework");
    global_logger().set_log_level(LOG_DEBUG);

    /* Parameters */
    std::string filename    = get_option_string("file", "data"); // Base filename
    int niters              = get_option_int("niters", 4);
    bool scheduler          = false;                    // Non-dynamic version of pagerank.

    std::map<int, std::set<int>> bundle_map = fetch_data(filename);

    /* Process input file - if not already preprocessed */
    int nshards             = convert_if_notexists<EdgeDataType>(filename, get_option_string("nshards", "auto"));

    /* Run */
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m);
    VertexRelabel program;
    engine.run(program, niters);
    metrics_report(m);

    // Find label counts for each bundle
    std::map<int, std::map<int, int>> bundle_label_counts =  km->getLabelCounts(bundle_map);

    // Vector of bundle ids, so we know what order our bundles are in
    std::vector<int> bundle_ids;
    int num_bundles = bundle_map.size();
    bundle_ids.reserve(num_bundles);
    for(auto const& elem: bundle_map)
        bundle_ids.push_back(elem.first);

    // Get all count arrays
    std::vector<std::vector<int>> count_arrays;
    bundle_ids.reserve(num_bundles);
    for (auto bundle_id : bundle_ids) {
        count_arrays.push_back(km->generate_count_array(bundle_label_counts.at(bundle_id)));
    }


    profile pf;

    // Create the distance matrix
    std::vector<double> distance_matrix;
    for (int i = 0 ; i < num_bundles; i++) {
        for (int j = 1; j < num_bundles - i; j++) {
            double distance = pf.calculate_distance(KULLBACKLEIBLER, count_arrays[i], count_arrays[i+j]);
            distance_matrix.push_back(distance);
        }
    }

    // Determine initial centroid value to use
    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> cluster_prior_results = kmeans_prior(num_bundles, distance_matrix);
    std::vector<std::vector<int>> cluster_prior = cluster_prior_results.first;

    //obtain the value of k for later clustering of distributions
    int total_number_of_valid_clusters_estimate = 0;
    for (std::vector<std::vector<int>>::iterator itr = cluster_prior.begin(); itr != cluster_prior.end(); itr++) {
        if (itr->size() > 0)
            total_number_of_valid_clusters_estimate++;
    }
    std::cout << "# of Clusters (estimate): " << total_number_of_valid_clusters_estimate << std::endl;

    //Initialize a vector that will hold the instance IDs of the ones that will be the initial centroild of the clustering of distributions
    std::vector<int> cluster_ids;

    //this vector contains for each cluster the instance that appears and the number of distance value of that instance
    //e.g. if cluster 0 has 1-0 1-2 1-4, then we have [(1 -> 3, 2 -> 1, 4 -> 1), <other_maps>]
    std::vector<std::map<int, int>> cluster_temps;

    for (std::vector<std::vector<int>>::iterator itr = cluster_prior.begin(); itr != cluster_prior.end(); itr++) {
        std::cout << "Prior Cluster: ";
        std::map<int, int> temp;
        for (std::vector<int>::iterator itr2 = itr->begin(); itr2 != itr->end(); itr2++) {
            for (int x = 0; x < num_bundles - 1; x++) {
                for (int y = 0; y < num_bundles - 1 - x; y++) {
                    if ((((( (num_bundles - 1) + ( num_bundles - x )) * x ) / 2 ) + y ) == *itr2) {
                        std::cout << x << "-" << x + 1 + y << " ";
                        std::pair<std::map<int,int>::iterator,bool> ret;
                        ret = temp.insert ( std::pair<int,int>(x,1) );
                        if (!ret.second) {
                            ret.first->second++;
                        }
                        ret = temp.insert ( std::pair<int,int>(x+1+y,1) );
                        if (!ret.second) {
                            ret.first->second++;
                        }
                    }
                }
            }
        }
        std::cout << std::endl;
        cluster_temps.push_back(temp);
    }

    for (size_t i = 0; i < cluster_temps.size(); i++) {
        if (cluster_temps[i].size() > 0) {
            int id = -1;
            int max_occur = -1;
            for (std::map<int, int>::iterator it = cluster_temps[i].begin(); it != cluster_temps[i].end(); it++) {
                if (it->second > max_occur) {
                    max_occur = it->second;
                    id = it->first;
                }
            }
            assert (id >= 0);
            assert (max_occur > 0);
            cluster_ids.push_back(id);
        }
    }

    std::vector<std::vector<int>> final_centroids;
    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> cluster_results = kmeans(total_number_of_valid_clusters_estimate, cluster_ids, count_arrays, final_centroids);

    std::vector<std::vector<int>> cluster = cluster_results.first;
    std::vector<std::vector<double>> cluster_distances = cluster_results.second;

    // Print out elements in a cluster
    for (std::vector<std::vector<int>>::iterator it = cluster.begin(); it != cluster.end(); it++) {
        std::cout << "Cluster: ";
        for (std::vector<int>::iterator itr2 = it->begin(); itr2 != it->end(); itr2++) {
            std::cout << bundle_ids[*itr2] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
