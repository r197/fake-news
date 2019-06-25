#ifndef helper_hpp
#define helper_hpp

#include <vector>
#include <utility>
#include <cassert>
#include <cmath>

double mean(std::vector<double> vec);

std::vector<double> count_distribution(std::vector<int> count_array, bool back_off);

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> kmeans_prior(int k, std::vector<double> distance_matrix);

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> kmeans(int k, std::vector<int>seeds, std::vector<std::vector<int>> count_array, std::vector<std::vector<int>>& centroids);

double calculate_distance(int method, std::vector<int> count_array1, std::vector<int> count_array2);
#endif /* helper_hpp */

