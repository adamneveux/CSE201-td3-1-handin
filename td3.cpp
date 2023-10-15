#include <iostream>
#include "td3.hpp"
#include "support.hpp"
#include <stdlib.h>
#include <math.h>       // sin, cos
#include <assert.h>

using namespace std;

using namespace support;

double* extend_array(double* array, int length, int new_size) {
    double* new_array = new double[new_size];
    for (int i=0; i<length; i++) {
        new_array[i] = array[i];
    }
    for (int i = length; i < new_size; i++) {
        new_array[i] = 0;
    }
    delete[] array;

  return new_array; // YOU CAN CHANGE THIS
}

double* shrink_array(double* array, int length, int new_size) {
    double* shrinked_arr = new double[new_size];
    for (int i=0; i<new_size; i++) shrinked_arr[i] = array[i];
    delete[] array;
    return shrinked_arr;
}

double* append_to_array(double element,
                        double* array,
                        int &current_size,
                        int &max_size) {
    int oneLeft = max_size - current_size;
    if (oneLeft == 0) {
        double* bigger_array = new double[max_size+5];
        for (int i=0; i<max_size; i++) bigger_array[i] = array[i];
        bigger_array[max_size] = element;
        current_size ++; max_size += 5;
        return bigger_array;
    }
    else {
        array[current_size] = element;
        current_size ++;
        return array;//is a pointer already
    }
}

double* remove_from_array(double* array,
                          int &current_size,
                          int &max_size) {
    current_size --; //with 1 element gone
    max_size = max_size - current_size >= 5 ? max_size -5 : max_size;
    double* new_smaller_array = new double[max_size];
    for (int i=0; i < current_size; i ++) {
        new_smaller_array[i] = array[i];
    }
    return new_smaller_array; // YOU CAN CHANGE THIS
}

bool simulate_projectile(const double magnitude, const double angle,
                         const double simulation_interval,
                         double *targets, int &tot_targets,
                         int *obstacles, int tot_obstacles,
                         double* &telemetry,
                         int &telemetry_current_size,
                         int &telemetry_max_size) {
  // YOU CAN MODIFY THIS FUNCTION TO RECORD THE TELEMETRY

  bool hit_target, hit_obstacle;
  double v0_x, v0_y, x, y, t;
  double PI = 3.14159265;
  double g = 9.8;

  v0_x = magnitude * cos(angle * PI / 180);
  v0_y = magnitude * sin(angle * PI / 180);

  t = 0;
  x = 0;
  y = 0;
  double* arrForTele[3] = {&t,&x,&y}; //new code

  hit_target = false;
  hit_obstacle = false;
  while (y >= 0 && (! hit_target) && (! hit_obstacle)) {
    double * target_coordinates = find_collision(x, y, targets, tot_targets);
    if (target_coordinates != NULL) {
      remove_target(targets, tot_targets, target_coordinates);
      hit_target = true;
    } else if (find_collision(x, y, obstacles, tot_obstacles) != NULL) {
      hit_obstacle = true;
    } else {
      t = t + simulation_interval;
      x = v0_x * t;
      y = v0_y * t  - 0.5 * g * t * t;
    }
    //new code:    
    for (int i =0; i < 3; i++) telemetry = append_to_array(*arrForTele[i], telemetry, telemetry_current_size, telemetry_max_size);
  }

    return hit_target;
}


//sub-function: given many arrays, find the soonest-value of them all, return a pointer to that array.
//DO NOT change the value of the index within the telemetry array of indices. 
//Let main fx do that.
int min_index(double* array_of_elements, int tot_teles) {
    int minInd = 0;
    for (int i=0; i < tot_teles; i ++) {
        if (array_of_elements[i] < array_of_elements[minInd]) {
            minInd = i;
        }
    }
    return minInd;
}

int index_of_array_with_value_to_add(int* inds_in_tele, double **teles, int tot_teles) {
    //indices of ind_in_tele align with teles.
    double* minTimes_array = new double[tot_teles];
    for (int i=0; i<tot_teles; i++) {
        minTimes_array[i] = teles[i][inds_in_tele[i]]; //inds_in_tele[i] is earliest value of non-added times in telemetry i.
    }
    return min_index(minTimes_array, tot_teles);
}

int sum_tele_sizes(int *tele_sizes, int tot_teles) {
    int sum=0;
    for (int i=0; i<tot_teles; i++) sum += tele_sizes[i];
    return sum;
}


void merge_telemetry(double **telemetries,
                     int tot_telemetries,
                     int *telemetries_sizes,
                     double* &global_telemetry,
                     int &global_telemetry_current_size,
                     int &global_telemetry_max_size) {
    /* 
    have indices incrementing, each meaning index of one sub-array (one telemetry). Do a mergesort kinda thing, but with multiple arrays involved. 
    Fit the 'best' each time into global_telemetry. To find the best: minimum time value (first index).
    */
    int* indices_in_telemetries = new int[tot_telemetries]; //index a of this represents an index telemetries[i][a], where all previous have been added alr.
    for (int i = 0; i < tot_telemetries; i++) indices_in_telemetries[i] = 0; //start at 0 of each array. JUMP by 3 each new increase.
    
    int tele_elements_to_add = sum_tele_sizes(telemetries_sizes, tot_telemetries);
    
    while (global_telemetry_current_size < tele_elements_to_add) {
        //find min pointer, add three-with-min to global, increment that pointer in ind_in_teles. repeat and rinse.
        int index_of_min_arr = index_of_array_with_value_to_add(indices_in_telemetries, telemetries, tot_telemetries);
        
        for (int i=0; i < 3; i++) {
            append_to_array(telemetries[index_of_min_arr][indices_in_telemetries[index_of_min_arr]], global_telemetry,
                            global_telemetry_current_size, global_telemetry_max_size);
        }
        
        indices_in_telemetries[index_of_min_arr] += 3;
    }
    
}
