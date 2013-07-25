// example1.cpp : Defines the entry point for the console application.
//
#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include "image.h"
#include "typedefs.h"


// Kernel that executes on the CUDA device
void render(const RenderSettings *settings,float3 *host_image, ROI roi, int gpu_no);
void print_device_properties();
void setGPU(int gpuNo);

int GRID_SIZE = 86400;
int WORKTAG = 1;
int DIETAG = 2;

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    //fprintf(stderr, "Hello world from process %d of %d\n", rank+1, size);
	
// render set-up
    int width = (argc > 1) ? atoi(argv[1]) : 360;
    int height = (argc > 2) ? atoi(argv[2]) : 240;
    int samples = (argc > 3) ? atoi(argv[3]) : 10;
    const char *name = (argc > 4) ? argv[3] : "output.bmp";
    RenderSettings settings = {width, height, samples, (char *)name};
    ROI roi = {0, width * height}; //legnth should be greater than size * width / samples
    const size_t host_size = settings.width * settings.height * sizeof (float3);
    float3 *host_image = (float3 *) malloc(host_size);
    unsigned i;
    for (i =0 ; i < roi.length; ++i) {
    	host_image[roi.startIdx + i].x = 0;
        host_image[roi.startIdx + i].y = 1;
        host_image[roi.startIdx + i].z = 0;
    }
	
    if (rank == 0) {
	    GRID_SIZE = width * height / (size - 1) ;
    	print_device_properties();
        // mpi master
        printf("\n--------------------------------------------------------------------------\n");
        printf("\033[34;01mRendering scene\033[0m - %d x %d, %d samples, %d GPUs\n", width, height, samples, size - 1);
        printf("--------------------------------------------------------------------------\n");
        const double start_time = MPI_Wtime();
        unsigned grid_pixel_size = GRID_SIZE;
        unsigned n_workitems = width*height / grid_pixel_size;
        ROI *regions = (ROI*)malloc(sizeof(ROI)*n_workitems); //NEWARR(ROI, n_workitems);
        unsigned cur_workitem = 0;
        while (cur_workitem < n_workitems) {
            regions[cur_workitem].startIdx = cur_workitem * grid_pixel_size;
            regions[cur_workitem].length   = grid_pixel_size;
            ++cur_workitem;
        }
        if (n_workitems * grid_pixel_size < width*height) { // we need one more
        	regions[cur_workitem].startIdx = cur_workitem * grid_pixel_size;
        	regions[cur_workitem].length = width*height - regions[cur_workitem].startIdx;
        	++n_workitems;
        	++cur_workitem;
        }
        // distribute ROIs to slaves
        // 1. send one unit of work to each slave
        cur_workitem = 0;
        unsigned active_procs = 0;
        unsigned dest_rank;
        for (dest_rank = 1; dest_rank < size; ++dest_rank) {
            // send next work item if we have it
            if (cur_workitem < n_workitems /* gotNext */) {
                //printf("Sending work item to %d\n", dest_rank);
                MPI_Send(&regions[cur_workitem++], 2, MPI_INT, dest_rank, WORKTAG, MPI_COMM_WORLD);
                active_procs++;
            }
            else {
                //printf("Killing process %d\n", dest_rank);
                MPI_Send(&regions[0], 2, MPI_INT, dest_rank, DIETAG, MPI_COMM_WORLD);
            }
        }
        // 2. wait for a slave to finish
        while (active_procs) {
            ROI roi;
            MPI_Status status;

            // receive the ROI of the computed block
            MPI_Recv(&roi, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            unsigned src = status.MPI_SOURCE;

            // receive the rendered bitmap data
            float3 *result = (float3*)malloc(sizeof(float3)*roi.length); 
            MPI_Recv(result, roi.length*sizeof(float3), MPI_BYTE, src, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // copy the received bitmap into our bigger bitmap
            unsigned i = 0, x, y;
            for (i = 0; i < roi.length; ++i) {
                host_image[roi.startIdx + i] = result[i];
            }
            free(result);

            // send next work item if we have it
            if (cur_workitem < n_workitems) {
                //fprintf(stderr, "Sending next work item to %d (%d remaining)\n", src, (n_workitems - cur_workitem));
                MPI_Send(&regions[cur_workitem++], 2, MPI_INT, src, WORKTAG, MPI_COMM_WORLD);
            }
            else {
                //fprintf(stderr, "Killing process %d\n", src);
                MPI_Send(&regions[0], 2, MPI_INT, src, DIETAG, MPI_COMM_WORLD);
                active_procs--;
            }
        }
        free(regions);
        printf("--------------------------------------------------------------------------\n");
        printf("Total time %f s.  " ,MPI_Wtime() - start_time);
        write_image_bmp(settings.output_file, host_image, settings.width, settings.height);
        free(host_image);
        MPI_Finalize();
        system("eog output.bmp");
        exit(0);
    }
    else {
        // mpi slave
	    setGPU(rank - 1);
        while(1) {
            // receive unit of work from master
            
            ROI roi = {0,0};
            MPI_Status status;
            //printf("Slave receiving ROI\n");
            MPI_Recv(&roi, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //printf("ok\n");
            if (status.MPI_TAG == DIETAG) {
                //printf("About to finalize...\n");
                MPI_Finalize();
                //printf("Finalized %d\n", rank);
                exit(0); // no more messages to process
            }
            else if (status.MPI_TAG == WORKTAG) {
                unsigned i;
            	for (i =0 ; i < roi.length; ++i) {
            		host_image[roi.startIdx + i].x = 1;
            		host_image[roi.startIdx + i].y = 1;
            		host_image[roi.startIdx + i].z = 0;
            	}
                render(&settings, host_image, roi, rank-1); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                // copy the result into a temp variable
                float3 *tiny = (float3*)malloc(sizeof(float3)*roi.length); //NEWARR(Color3, w*h);


                for (i = 0; i < roi.length; ++i) {
                    tiny[i] = host_image[roi.startIdx + i];
                }

                // send the result back
                MPI_Send(&roi, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Send(tiny, roi.length*sizeof(float3), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
                free(tiny);
            }
            else exit(1);
        }
    }

    return EXIT_SUCCESS;
}
