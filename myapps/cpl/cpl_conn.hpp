//
// Created by Rachel on 6/20/2019.
//

#ifndef cpl_conn_hpp
#define cpl_conn_hpp

#include <backends/cpl-odbc.h>
#include <cpl-db-backend.h>
#include <cpl-exception.h>
#include <cpl.h>
#include <cplxx.h>
#include <iostream>
#include <string>
#include <vector>

void connect_cpl();

void get_all_bundles(const char* prefix, std::vector<cplxx_object_info> *bundles);

void get_bundle_objects(cpl_id_t bundle_id, std::vector<cplxx_object_info> *objects);

void get_bundle_relations(cpl_id_t bundle_id, std::vector<cpl_relation_t> *relations);

std::string get_object_type(const char* prefix, cpl_id_t bundle_id, cpl_id_t object_id);

#endif /* cpl_conn_hpp */
