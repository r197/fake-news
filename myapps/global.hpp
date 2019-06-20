//
//  global.h
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#ifndef global_hpp
#define global_hpp

#include <vector>
#include <map>

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
struct type_label {
    int old_src;
    int old_dst;
    int new_src;
    int new_dst;
    int edge;
};


typedef int VertexDataType;
typedef type_label EdgeDataType;//src_type dst_type edge_type

struct monitor_profile {
    std::vector<int> count_array;
    std::map<int, int> label_map;
};

typedef monitor_profile monitor_profile;

//monitor_profile monitored;


#endif /* global_hpp */
