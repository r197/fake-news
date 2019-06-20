//
//  vertex.hpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#ifndef vertex_hpp
#define vertex_hpp

#include <mutex>
#include "graphchi_basic_includes.hpp"
#include "../kernel_maps/kernel_maps.hpp"
#include "../global.hpp"

using namespace graphchi;

struct VertexRelabel : public GraphChiProgram<VertexDataType, EdgeDataType> {

    //get the singleton kernelMaps
    KernelMaps* km = KernelMaps::get_instance();

    //locks for sync update
    std::mutex relabel_map_lock;
    std::mutex label_map_lock;

    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext);

    void before_iteration(int iteration, graphchi_context &gcontext);

    void after_iteration(int iteration, graphchi_context &gcontext);

    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext);

    void after_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext);
};

#endif /* vertex_relabel_hpp */
