//
//  main.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#include <string>
#include <iostream>
#include <fstream>
#include "vertex_relabeling/vertex_relabel.hpp"
#include "cpl/cpl_conn.hpp"
#include "profile/profile.hpp"

#define KULLBACKLEIBLER 0
#define HELLINGER 1
#define EUCLIDEAN 2

const char* prefix = "test";

// Get all the bundle objects and relations from the cpl
std::map<int, std::set<int>> fetch_data(std::string filename, std::vector<int> bundle_ids) {
    std::ofstream outfile (filename);
    std::map<int, std::set<int>> bundle_map;

    std::map<int, std::string> object_types; // Map of original object ID to object type
    std::map<int, int> new_object_indices; //Map of original object ID to new object ID
    int curr_index = 0;

    for (const auto& bundle_id : bundle_ids) {
        std::set<int> all_new_object_indices; // Keeps track of which objects are in this bundle

        // Get the objects in the bundle
        auto *objects = new std::vector<cplxx_object_info>();
        get_bundle_objects(bundle_id, objects);

        for (const auto& object : *objects) {
            auto it = object_types.find(object.id);
            if (it == object_types.end()) {
                std::string type = get_object_type(prefix, bundle_id, object.id);
                object_types.insert(std::pair<int,std::string>(object.id, type));
            }
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
        bundle_map.insert(std::pair<int,std::set<int>>(bundle_id, all_new_object_indices));
    }
    outfile.close();
    return bundle_map;
}

void writeBundleIdFile(std::string fileName, std::vector<int> bundleIds) {
    std::ofstream file;
    file.open(fileName);
    for (auto bundle: bundleIds) {
        file << std::to_string(bundle) << "\n";
    }
    file.close();
}

void readBundleIdFile(std::string fileName, std::vector<int> &bundleIds) {
    std::fstream f(fileName, std::ios_base::in);
    int id;
    while (f >> id) {
        bundleIds.push_back(id);
    }
}

void writeBundleMapFile(std::string fileName, std::map<int, std::set<int>> bundleMap) {
    std::ofstream file;
    file.open(fileName);
    for (auto const& entry : bundleMap) {
        file << std::to_string(entry.first) << ":";
        std::string padding;
        for (auto const e : entry.second) {
            file << padding << std::to_string(e);
            padding = ", ";
        }
        file << "\n";
    }
    file.close();
}

void readBundleMapFile(std::string fileName, std::map<int, std::set<int>> &bundleMap) {
    std::ifstream file(fileName);
    std::string line;
    std::string token;
    while (std::getline(file, line)) {
        size_t pos = 0;
        int index = line.find(':');
        int first = stoi(line.substr(0, index));
        std::set<int> second;
        line.erase(0, index + 1);
        while ((pos = line.find(',')) != std::string::npos) {
            second.insert(stoi(line.substr(0, pos)));
            line.erase(0, pos + 1);
        }
        if (!line.empty())
            second.insert(stoi(line));
        bundleMap.insert(std::pair<int,std::set<int>>(first, second));
    }
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

// Cluster the label counts for the bundles
std::vector<std::vector<int>> clusterBundles(std::vector<std::vector<int>> count_arrays) {
    profile pf;
    int num_bundles = count_arrays.size();

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
    return cluster_results.first;
}

// Get a count array for each bundle
std::vector<std::vector<int>> getCountArrays(
        std::map<int, std::map<int, int>> bundle_label_counts,
        std::vector<int> bundle_ids) {
    KernelMaps* km = KernelMaps::get_instance();

    std::vector<std::vector<int>> count_arrays;
    count_arrays.reserve(bundle_ids.size());
    for (auto bundle_id : bundle_ids) {
        count_arrays.push_back(km->generate_count_array(bundle_label_counts.at(bundle_id)));
    }
    return count_arrays;
}

int main(int argc, const char ** argv) {
    // Create the single instance of KernelMap
    KernelMaps* km = KernelMaps::get_instance();
    km->resetMaps();

    // Connect to the cpl
    connect_cpl();

    graphchi_init(argc, argv);
    metrics m("Detection Framework");
    global_logger().set_log_level(LOG_DEBUG);

    // Parameters
    std::string data_directory    = get_option_string("outputDir", "."); // Base filename
    int niters              = get_option_int("niters", 4);
    std::string bundle_file = get_option_string("bundles", "");
    std::string existing_data_dir = get_option_string("existingDir", "");
    bool scheduler          = false;                    // Non-dynamic version of pagerank.

    std::vector<int> bundle_ids;
    std::map<int, std::set<int>> bundle_map;
    std::string filename;

    if (existing_data_dir.empty()) {

        // Read the bundle file to see which bundles we want to analyze
        auto *bundles = new std::vector<cplxx_object_info>();
        if (bundle_file.empty()){
            std::cout << "No bundle file specified. Analyzing all bundles" << std::endl;
            get_all_bundles(prefix, bundles);
            bundle_ids.reserve(bundles->size());
            for (const auto bundle : *bundles) {
                bundle_ids.push_back(bundle.id);
            }
        } else {
            std::cout << "Extracting bundle IDs from the bundle file" << std::endl;
            std::fstream f(bundle_file, std::ios_base::in);
            int id;
            while (f >> id) {
                bundle_ids.push_back(id);
            }
        }

        filename = data_directory + "/edgeList";

        // Get the objects & relations for all the bundles
        bundle_map = fetch_data(filename, bundle_ids);

        writeBundleIdFile(data_directory + "/bundleIDs", bundle_ids);
        writeBundleMapFile(data_directory + "/bundleMap", bundle_map);
    } else {
        std::cout << "Data files were passed in. Analyzing the passed in data" << std::endl;
        filename = existing_data_dir + "/edgeList";
        readBundleIdFile(existing_data_dir + "/bundleIDs", bundle_ids);
        readBundleMapFile(existing_data_dir + "/bundleMap", bundle_map);
    }

    // Process input file - if not already preprocessed
    int nshards = convert_if_notexists<EdgeDataType>(filename, get_option_string("nshards", "auto"));

    // Run
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m);
    VertexRelabel program;
    engine.run(program, niters);

    // Find label counts for each bundle
    std::map<int, std::map<int, int>> bundle_label_counts =  km->getLabelCounts(bundle_map);

    // Get all count arrays
    std::vector<std::vector<int>> count_arrays = getCountArrays(bundle_label_counts, bundle_ids);

    // Cluster the count arrays
    std::vector<std::vector<int>> cluster = clusterBundles(count_arrays);

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
