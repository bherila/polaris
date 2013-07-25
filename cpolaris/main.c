#include <stdio.h>
#include <stdlib.h>
#include "scene.h"
#include "camera.h"
#include "sampler.h"
#include "renderer.h"
#include "image.h"
#include "scenes.h"
#include "vector.h"
#include <sys/time.h>
#include <sys/times.h>
#include <assert.h>

//#include "kdtree.h"
//#include <GL/glut.h>

#define USE_MPI
#ifdef USE_MPI
#   include <mpi.h>
#   define WORKTAG  1
#   define DIETAG   2
#endif

Scene *sc;

#define MIN(x,y) (x)<(y)?(x):(y)
#define MAX(x,y) (x)>(y)?(x):(y)

int   NUM_SAMPLES = 60;           // number of super samples
char* OUTFILE     = "output.bmp";  // output filename
int   RS_DEPTH    = 3;             // recursive spheres depth
int   IMG_WIDTH   = 7680;
int   IMG_HEIGHT  = 4320;
int   GRID_SIZE   = 240;
float EMITTER_Z   = -1.48f;
float EMITTER_POWER = 0.5f; //3.5

int main(int argc, char** argv) {
fprintf(stderr, "usage: a.out NUMSAMPLES OUTFILE\n");
#ifdef USE_MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    fprintf(stderr, "Hello world from process %d of %d\n", rank+1, size);
#endif

    //Select the render settings
    //EMITTER_POWER = atof(argv[1]);
    NUM_SAMPLES   = atoi(argv[1]);
	  OUTFILE       = argv[2];

    printf("%d NUM SAMPLES, outfile %s\n", NUM_SAMPLES, OUTFILE);
    RenderSettings rs;
    rs.mROI.mXLeft = rs.mROI.mYTop = 0;
    rs.mWidth = rs.mROI.mXRight = IMG_WIDTH;
    rs.mHeight = rs.mROI.mYBottom = IMG_HEIGHT;
    rs.mAASamples = NUM_SAMPLES;
    rs.mThreads = 1;
    printf("%d NUM SAMPLES\n", NUM_SAMPLES);

    //Scene *scene = generate_diffuse_cube(rs);
    //Scene *scene = generate_9_spheres(rs);
    //Scene *scene = generate_cornell_box(rs);
    //int ap = RS_DEPTH;
    //Scene *scene = generate_recursive_spheres(rs, ap);
    Scene *scene = generate_diffuse_spheres(rs, EMITTER_POWER);
    sc = scene;
    //kdtree_test(scene, argc, argv);
    fprintf(stderr, "AAAA %d\n", rank);
    //Render
    Sampler *s = createSampler();
    Color3 *image = (Color3*)malloc(rs.mWidth * rs.mHeight * sizeof (Color3));
    struct tms tm;

#ifdef USE_MPI

    if (rank == 0) {
        // mpi master
        printf("Master starting\n");
        const double start_time = MPI_Wtime();
        unsigned grid_pixel_size = GRID_SIZE;
        unsigned nsquares_x = rs.mWidth / (double)grid_pixel_size + 0.5;
        unsigned nsquares_y = rs.mHeight / (double)grid_pixel_size + 0.5;
        unsigned n_workitems = nsquares_x * nsquares_y;
        ROI *regions = (ROI*)malloc(sizeof(ROI)*n_workitems); //NEWARR(ROI, n_workitems);
        unsigned cur_workitem = 0;
        unsigned x,y;
        for (y = 0; y < rs.mHeight; y += grid_pixel_size) {
            for (x = 0; x < rs.mWidth; x += grid_pixel_size) {
                regions[cur_workitem].mYTop = y;
                regions[cur_workitem].mYBottom = MIN(y + grid_pixel_size, rs.mHeight) - 1;
                regions[cur_workitem].mXLeft = x;
                regions[cur_workitem].mXRight = MIN(x + grid_pixel_size, rs.mWidth) - 1;
                //TODO: regions[cur_workitem].print(); printf("\n");
                ++cur_workitem;
            }
        }
        // distribute ROIs to slaves
        // 1. send one unit of work to each slave
        cur_workitem = 0;
        unsigned active_procs = 0;
        unsigned rank;
        for (rank = 1; rank < size; ++rank) {
            // send next work item if we have it
            if (cur_workitem < n_workitems /* gotNext */) {
                printf("Sending work item to %d\n", rank);
                MPI_Send(regions[cur_workitem++].data, 4, MPI_UNSIGNED, rank, WORKTAG, MPI_COMM_WORLD);
                active_procs++;
            }
            else {
                printf("Killing process %d\n", rank);
                MPI_Send(0, 0, MPI_INT, rank, DIETAG, MPI_COMM_WORLD);
            }
        }
        // 2. wait for a slave to finish
        while (active_procs) {
            ROI roi;
            MPI_Status status;

            // receive the ROI of the computed block
            MPI_Recv(&roi, 4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            unsigned src = status.MPI_SOURCE;
            unsigned w   = roi.mXRight - roi.mXLeft + 1;
            unsigned h   = roi.mYBottom - roi.mYTop + 1;

            // receive the rendered bitmap data
            Color3 *result = (Color3*)malloc(sizeof(Color3)*w*h); //NEWARR(Color3, w*h);
            MPI_Recv(result, w*h*sizeof(Color3), MPI_BYTE, src, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // copy the received bitmap into our bigger bitmap
            unsigned i = 0, x, y;
            for (y = roi.mYTop; y < roi.mYBottom; ++y) {
                for (x = roi.mXLeft  ; x < roi.mXRight; ++x) {
                    image[y*rs.mWidth+x] = result[i++];
                }
                i++;
            }
            assert(result);
            free(result);
            result = 0;

            // send next work item if we have it
            if (cur_workitem < n_workitems) {
                fprintf(stderr, "Sending next work item to %d (%d remaining)\n", src, (n_workitems - cur_workitem));
                MPI_Send(regions[cur_workitem++].data, 4, MPI_UNSIGNED, src, WORKTAG, MPI_COMM_WORLD);
            }
            else {
                fprintf(stderr, "Killing process %d\n", rank);
                MPI_Send(0, 0, MPI_INT, src, DIETAG, MPI_COMM_WORLD);
                active_procs--;
            }
        }
        assert(regions);
        free(regions);
        regions = 0;

        times(&tm);
        //printf("\033[31mRender complete.\033[0m\n\n");
        fprintf(stdout, "Samples: %d\tThreads: %d   Total time: %'.2f s  Traversals: %'.2f M/s\n", rs.mAASamples, size,  MPI_Wtime() - start_time);
        write_image_bmp(OUTFILE, image, rs.mWidth, rs.mHeight);
		    exit(0);
    }
    else {
        // mpi slave
        while(1) {
            // receive unit of work from master
            ROI roi = {0,0,0,0};
            MPI_Status status;
            MPI_Recv(roi.data, 4, MPI_UNSIGNED, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == DIETAG) {
                printf("Rank %d terminating\n", rank);
                //MPI_Finalize();
                printf("Finalized\n");
                exit(0); // no more messages to process
            }
            else if (status.MPI_TAG == WORKTAG) {
                unsigned w = roi.right -  roi.left + 1;
                unsigned h = roi.bottom - roi.top + 1;
                //printf("Process %d rendering [%d x %d]", rank, w, h); /*roi.print();*/ printf("\n");

                unsigned i = 0, x, y;

                // do the work
                //TODO:
                //RenderThread *t = new RenderThreadQt(r, image);
                //t->setROI(roi.mXLeft, roi.right, roi.top, roi.bottom);
                //t->run();
                rs.mROI = roi;
                //ray_trace(scene, s, image, &rs);
                path_trace(scene, s, image, &rs);

                // copy the result into a temp variable
                Color3 *tiny = (Color3*)malloc(sizeof(Color3)*w*h); //NEWARR(Color3, w*h);
                i = 0;
                for (y = roi.top;  y <= roi.bottom; ++y)
                    for (x = roi.left; x <= roi.right; ++x)
                        tiny[i++] = image[y*rs.mWidth+x];
                assert(i==w*h);

                // send the result back
                MPI_Send(roi.data, 4, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
                MPI_Send(tiny, w*h*sizeof(Color3), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
                free(tiny); //DELARR(tiny);
            }
            else exit(1);
        }
    }
#else

    printf("\033[34;01mcPolaris 1.0 Renderer\033[0m\n\n\033[32mRendering %d objects at %d x %d with %d samples per pixel...\033[0m\n\n",
            scene->mNumObjects, rs.mWidth, rs.mHeight, rs.mAASamples);

    // normal single-threaded mode
    ray_trace(scene, s, image, &rs);
    //path_trace(scene, s, image, &settings);

    times(&tm);
    printf("\033[31mRender complete.\033[0m\n");
    printf("\nRays fired: %'.2f M   Total time: %'.2f s  Traversals: %'.2f M/s\n\n", scene->mNumrays / 1000000.0f,  tm.tms_utime / 100.0f,
            (scene->mNumrays / (tm.tms_utime / 100.0f) / 1000000.0f));
    write_image_bmp(OUTFILE, image, rs.mWidth, rs.mHeight);


    printf("\n");

#endif

    //Cleanup
    //releaseScene(scene);
    //releaseSampler(s);
    //free(image);

    return (EXIT_SUCCESS);
}

//void kdtree_render(int node) {
//    kdNode node = sc->mTree->nodes[node];
//    if (isLeaf) {
//    }
//    else {
//        // render the children
//        int c1 = node.value;
//        int c2 = node.value+1;
//
//
//
//        kdtree_render(c1);
//        kdtree_render(c2);
//    }
//}
//
//void kdtree_render() {
//
//}
//
//void kdtree_test(Scene* scene, int argc, char** argv) {
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//    glutCreateWindow("kd-tree");
//    glutDisplayFunc(kdtree_render);
//    init();
//    glutMainLoop();
//    return 0;
//}
