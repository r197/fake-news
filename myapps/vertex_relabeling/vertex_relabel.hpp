//
// Created by Rachel on 6/20/2019.
//

#ifndef vertex_hpp
#define vertex_hpp

#include <mutex>
#include "graphchi_basic_includes.hpp"
#include "../kernel_maps/kernel_maps.hpp"

using namespace graphchi;

struct type_label {
    int old_src;
    int old_dst;
    int new_src;
    int new_dst;
    int edge;
};

typedef int VertexDataType;
typedef type_label EdgeDataType;

struct VertexRelabel : public GraphChiProgram<VertexDataType, EdgeDataType> {

    //get the singleton kernelMaps
    KernelMaps* km = KernelMaps::get_instance();

    //locks for sync update
    std::mutex relabel_map_lock;
    std::mutex label_map_lock;

    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext);

};

#endif /* vertex_relabel_hpp */
