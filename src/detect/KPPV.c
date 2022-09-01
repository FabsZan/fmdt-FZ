/*
 * Copyright (c) 2022, Clara Ciocan/ Mathuran Kandeepan
 * LIP6
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <nrc2.h>

#include "fmdt/tools.h"
#include "fmdt/KPPV.h"

#define INF32 0xFFFFFFFF
#define MAX_DIST 100

KKPV_data_t* KPPV_alloc_and_init_data(int i0, int i1, int j0, int j1) {
    KKPV_data_t* data = (KKPV_data_t*)malloc(sizeof(KKPV_data_t));
    data->i0 = i0;
    data->i1 = i1;
    data->j0 = j0;
    data->j1 = j1;
    data->nearest = (uint32_t**)ui32matrix(data->i0, data->i1, data->j0, data->j1);
    data->distances = (float**)f32matrix(data->i0, data->i1, data->j0, data->j1);
    data->conflicts = (uint32_t*)ui32vector(data->j0, data->j1);
    zero_ui32matrix(data->nearest, data->i0, data->i1, data->j0, data->j1);
    zero_f32matrix(data->distances, data->i0, data->i1, data->j0, data->j1);
    zero_ui32vector(data->conflicts, data->j0, data->j1);
    return data;
}

void KPPV_free_data(KKPV_data_t* data) {
    free_ui32matrix(data->nearest, data->i0, data->i1, data->j0, data->j1);
    free_f32matrix(data->distances, data->i0, data->i1, data->j0, data->j1);
    free_ui32vector(data->conflicts, data->j0, data->j1);
    free(data);
}

void _compute_distance(const float* ROI0_x, const float* ROI0_y, const uint32_t* ROI0_S, const size_t n_ROI0,
                       const float* ROI1_x, const float* ROI1_y, const uint32_t* ROI1_S, const size_t n_ROI1,
                       float** distances) {
    // parcours des stats 0
    for (size_t i = 0; i < n_ROI0; i++) {
        if (ROI0_S[i] > 0) {
            float x0 = ROI0_x[i];
            float y0 = ROI0_y[i];

            // parcours des stats 1
            for (size_t j = 0; j < n_ROI1; j++) {
                if (ROI1_S[j] > 0) {
                    float x1 = ROI1_x[j];
                    float y1 = ROI1_y[j];

                    // distances au carré
                    float d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);

                    // if d > MAX_DIST, on peut economiser l'accès mémoire (a implementer)
                    distances[i][j] = d;
                }
            }
        }
    }
}

void compute_distance(float** distances, const ROI_t* ROI_array0, const ROI_t* ROI_array1) {
    _compute_distance(ROI_array0->x, ROI_array0->y, ROI_array0->S, ROI_array0->_size, ROI_array1->x, ROI_array1->y,
                      ROI_array1->S, ROI_array1->_size, distances);
}

void KPPV_match1(const float* ROI0_x, const float* ROI0_y, const uint32_t* ROI0_S, const size_t n_ROI0,
                 const float* ROI1_x, const float* ROI1_y, const uint32_t* ROI1_S, const size_t n_ROI1,
                 uint32_t** nearest, float** distances, uint32_t* conflicts, const int k) {
    int k_index, val, cpt = 0;

    // vecteur de conflits pour debug
    // zero_ui32vector(conflicts, 0, ROI_array1->size);

    zero_ui32matrix(nearest, 0, n_ROI0, 0, n_ROI1);

    // calculs de toutes les distances euclidiennes au carré entre nc0 et nc1
    _compute_distance(ROI0_x, ROI0_y, ROI0_S, n_ROI0, ROI1_x, ROI1_y, ROI1_S, n_ROI1, distances);

    // les k plus proches voisins dans l'ordre croissant
    for (k_index = 1; k_index <= k; k_index++) {
        // parcours des distances
        for (size_t i = 0; i < n_ROI0; i++) {
            for (size_t j = 0; j < n_ROI1; j++) {
                // if une distance est calculée et ne fait pas pas déjà parti du tab nearest
                if ((distances[i][j] != INF32) && (nearest[i][j] == 0) && (distances[i][j] < MAX_DIST)) {
                    val = distances[i][j];
                    cpt = 0;
                    // // compte le nombre de distances < val
                    for (size_t l = 0; l < n_ROI1; l++) {
                        if ((distances[i][l] < val) && (distances[i][l] != INF32)) {
                            cpt++;
                        }
                    }
                    // k_index-ième voisin
                    if (cpt < k_index) {
                        nearest[i][j] = k_index;
                        // vecteur de conflits
                        // if (k_index == 1){
                        //         conflicts[j]++;
                        // }
                        break;
                    }
                }
            }
        }
    }
}

void KPPV_match2(const uint32_t** nearest, const float** distances, const uint16_t* ROI0_id, int32_t* ROI0_next_id,
                 const size_t n_ROI0, const uint16_t* ROI1_id, int32_t* ROI1_prev_id, const size_t n_ROI1) {
    uint32_t rang = 1;
    for (size_t i = 0; i < n_ROI0; i++) {
    change:
        for (size_t j = 0; j < n_ROI1; j++) {
            // si pas encore associé
            if (!ROI1_prev_id[j]) {
                // si ROI_array1->data[j] est dans les voisins de ROI0
                if (nearest[i][j] == rang) {
                    float d = distances[i][j];
                    // test s'il existe une autre CC de ROI0 de mm rang et plus proche
                    for (size_t k = i + 1; k < n_ROI0; k++) {
                        if (nearest[k][j] == rang && distances[k][j] < d) {
                            rang++;
                            goto change;
                        }
                    }
                    // association
                    ROI0_next_id[i] = ROI1_id[j];
                    ROI1_prev_id[j] = ROI0_id[i];
                    break;
                }
            }
        }
        rang = 1;
    }
}

void _KPPV_match(KKPV_data_t* data, const uint16_t* ROI0_id, const float* ROI0_x, const float* ROI0_y,
                 const uint32_t* ROI0_S, int32_t* ROI0_next_id, const size_t n_ROI0, const uint16_t* ROI1_id,
                 const float* ROI1_x, const float* ROI1_y, const uint32_t* ROI1_S, int32_t* ROI1_prev_id,
                 const size_t n_ROI1, const int k) {
    KPPV_match1(ROI0_x, ROI0_y, ROI0_S, n_ROI0, ROI1_x, ROI1_y, ROI1_S, n_ROI1, data->nearest, data->distances,
                data->conflicts, k);
    KPPV_match2((const uint32_t**)data->nearest, (const float**)data->distances, ROI0_id, ROI0_next_id, n_ROI0, ROI1_id,
                ROI1_prev_id, n_ROI1);
}

void KPPV_match(KKPV_data_t* data, ROI_t* ROI_array0, ROI_t* ROI_array1, const int k) {
    _KPPV_match(data, ROI_array0->id, ROI_array0->x, ROI_array0->y, ROI_array0->S, ROI_array0->next_id,
                ROI_array0->_size, ROI_array1->id, ROI_array1->x, ROI_array1->y, ROI_array1->S, ROI_array1->prev_id,
                ROI_array1->_size, k);
}

void KPPV_save_asso(const char* filename, const uint32_t** nearest, const float** distances, ROI_t* ROI_array) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "(EE) error ouverture %s \n", filename);
        exit(1);
    }

    // tmp (le temps de mettre à jour n)
    int cpt = 0;
    for (size_t i = 0; i < ROI_array->_size; i++) {
        if (ROI_array->S[i] != 0 && ROI_array->next_id[i])
            cpt++;
    }
    if (cpt != 0) {

        fprintf(f, "%d\n", cpt);

        size_t j;
        for (size_t i = 0; i < ROI_array->_size; i++) {
            if (!ROI_array->next_id[i]) {
                if (ROI_array->S[i] > 0)
                    fprintf(f, "%4lu \t ->   pas d'association\n", i);
            } else {
                j = (size_t)(ROI_array->next_id[i] - 1);
                fprintf(f, "%4lu \t -> %4lu \t  : distance = %10.2f \t ; %4d-voisin\n", i, j, distances[i][j],
                        nearest[i][j]);
            }
        }
    }
    fclose(f);
}

void KPPV_save_asso_VT(const char* filename, int nc0, ROI_t* ROI_array, int frame) {
    FILE* f = fopen(filename, "a");
    if (f == NULL) {
        fprintf(stderr, "(EE) error ouverture %s \n", filename);
        exit(1);
    }
    fprintf(f, "%05d_%05d\n", frame, frame + 1);

    size_t j;
    for (size_t i = 0; i <= ROI_array->_size; i++) {
        if (!ROI_array->next_id[i]) {
            if (ROI_array->S[i] > 0)
                fprintf(f, "%4lu \t ->   pas d'association\n", i);
        } else {
            j = (size_t)(ROI_array->next_id[i] - 1);
            fprintf(f, "%4lu \t -> %4lu \n", i, j);
        }
    }
    fprintf(f, "-------------------------------------------------------------------------------------------------------"
               "-----\n");
    fclose(f);
}

void KPPV_save_conflicts(const char* filename, uint32_t* conflicts, uint32_t** nearest, float** distances, int n_asso,
                         int n_conflict) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "(EE) error ouverture %s \n", filename);
        exit(1);
    }

    // tmp (le temps de mettre à jour n)
    int cpt = 0;
    for (int i = 1; i <= n_conflict; i++) {
        if (conflicts[i] != 1 && conflicts[i] != 0)
            cpt++;
    }
    if (cpt != 0) {

        fprintf(f, "%d\n", cpt);

        for (int j = 1; j <= n_conflict; j++) {
            if (conflicts[j] != 1 && conflicts[j] != 0) {
                fprintf(f, "conflit CC = %4d : ", j);
                for (int i = 1; i <= n_asso; i++) {
                    if (nearest[i][j] == 1) {
                        fprintf(f, "%4d\t", i);
                    }
                }
                fprintf(f, "\n");
            }
        }
    }
    fclose(f);
}

void KPPV_save_asso_conflicts(const char* path, const int frame, const KKPV_data_t* data, const ROI_t* ROI_array0,
                              const ROI_t* ROI_array1, const track_t* track_array) {
    assert(frame >= 0);
    char filename[1024];

    sprintf(filename, "%s/%05d_%05d.txt", path, frame, frame + 1);

    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "(EE) error ouverture %s \n", filename);
        exit(1);
    }

    // stats
    fprintf(f, "# Frame n°%05d (cur)\n", frame);
    features_save_stats_file(f, ROI_array0, track_array, 1);
    fprintf(f, "#\n# Frame n°%05d (next)\n", frame + 1);
    features_save_stats_file(f, ROI_array1, track_array, 0);
    fprintf(f, "#\n");

    // Asso
    int cpt = 0;
    for (size_t i = 0; i < ROI_array0->_size; i++) {
        if (ROI_array0->next_id[i] != 0)
            cpt++;
    }
    fprintf(f, "# Associations [%d]:\n", cpt);
    size_t j;

    if (cpt) {
        double mean_error = features_compute_mean_error(ROI_array0);
        double std_deviation = features_compute_std_deviation(ROI_array0, mean_error);
        fprintf(f, "# * mean error    = %.3f\n", mean_error);
        fprintf(f, "# * std deviation = %.3f\n", std_deviation);

        fprintf(f, "# ------------||---------------||------------------------\n");
        fprintf(f, "#    ROI ID   ||    Distance   ||          Error         \n");
        fprintf(f, "# ------------||---------------||------------------------\n");
        fprintf(f, "# -----|------||--------|------||-------|-------|--------\n");
        fprintf(f, "#  cur | next || pixels | k-nn ||    dx |    dy |      e \n");
        fprintf(f, "# -----|------||--------|------||-------|-------|--------\n");
    }

    for (size_t i = 0; i < ROI_array0->_size; i++) {
        if (ROI_array0->S[i] == 0)
            continue;
        if (ROI_array0->next_id[i]) {
            j = (size_t)(ROI_array0->next_id[i] - 1);
            float dx = ROI_array0->dx[i];
            float dy = ROI_array0->dy[i];
            fprintf(f, "  %4lu | %4lu || %6.2f | %4d || %5.1f | %5.1f | %6.3f \n", i, j, data->distances[i][j],
                    data->nearest[i][j], dx, dy, ROI_array0->error[i]);
        }
    }

    fprintf(f, "#\n");
    fprintf(f, "# tracks [%lu]:\n", track_array->_size);
    if (track_array->_size)
        tracking_print_track_array(f, track_array);

    // // Conflicts
    // cpt = 0;
    // for(int i = 1; i<= n_conflict; i++){
    //     if(conflicts[i] != 1 && conflicts[i] != 0)
    //         cpt++;
    // }

    // fprintf(f, "Conflicts\n%d\n", cpt);

    // for(int j = 1; j <= n_conflict; j++){
    //     if (conflicts[j] != 1 && conflicts[j] != 0){
    //         fprintf(f, "conflit CC = %4d : ", j);
    //         for(int i = 1 ; i <= ROI_array0->_size; i++){
    //             if (nearest[i][j] == 1 ){
    //                 fprintf(f, "%4d\t", i);
    //             }
    //         }
    //         fprintf(f, "\n");
    //     }
    // }
    fclose(f);
}