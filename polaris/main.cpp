#include "stdafx.h"
#include <stdio.h>

#include <iostream>
#include "scene.h"
#include "renderer.h"
#include "renderthread.h"

#define USE_GD

#ifdef USE_GD
#   include <gd.h>
#endif
#ifdef USE_QIMAGE
#   include <QtGui/QImage>
#   include <QImage>
#endif

using namespace std;



//#define USE_MPI
//#define QTHREAD
#define NO_MT

#ifdef QTHREAD
#   include <QThreadPool>
#endif

#ifdef USE_MPI
#   include <mpi.h>
#   define WORKTAG  1
#   define DIETAG   2
#endif

int main(int argc, char **argv) {
    cout << "POLARIS 1.0" << endl;
    //Vector4 v1(1,1,1,1);
    //cout << (v1*2) << endl;

    RenderSettings rs;
    rs.aasamples = 100;
    rs.width = 250;
    rs.height = 250;
    rs.threads = 1;
    cout << "Set up renderer" << endl;

    Camera *cam = new PerspectiveCamera();
    cam->setNear(0.1);
    cam->setFar(100);
    //cam->orient(Vector4(0,0.185,-0.5,1), Vector3(0,0,-1), Vector3(0,1,0));
    cam->orient(Vector4(0,0,3,1), Vector3(0,0,0), Vector3(0,1,0));
    cam->setHeightAngle(45);
    cam->setAspectRatio(rs.width/ (real_t)rs.height);
    cout << "Set up camera" << endl;

    Scene *s = new Scene();
    s->setCamera(cam);
    s->parseFile("scenes/general/cornellbox.xml");
    s->init();

    Renderer *r = new RayTracer(s, &rs);
    //Renderer *r = new PathTracer(s, &rs);

    Color3 *image = new Color3[rs.width * rs.height];

#ifdef USE_MPI
    int rank, size;
    MPI_Init (&argc, &argv); /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank); /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size); /* get number of processes */
    fprintf( stdout,"Hello world from process %d of %d\n",rank, size );

    if (rank == 0) {
        // mpi master
        unsigned grid_pixel_size = 250;
        unsigned nsquares_x = rs.width  / (real_t)grid_pixel_size + (real_t)0.5;
        unsigned nsquares_y = rs.height / (real_t)grid_pixel_size + (real_t)0.5;
        unsigned n_workitems = nsquares_x * nsquares_y;
        ROI *regions = NEWARR(ROI, n_workitems);
        unsigned cur_workitem = 0;
        for (unsigned y = 0; y < rs.height; y += grid_pixel_size) {
            for (unsigned x = 0; x < rs.width; x += grid_pixel_size) {
                regions[cur_workitem].top = y;
                regions[cur_workitem].bottom = min(y + grid_pixel_size, rs.height) - 1;
                regions[cur_workitem].left = x;
                regions[cur_workitem].right = min(x + grid_pixel_size, rs.width) - 1;
                regions[cur_workitem].print(); printf("\n");
                ++cur_workitem;
            }
        }
        // distribute ROIs to slaves
        // 1. send one unit of work to each slave
        cur_workitem = 0;
        unsigned active_procs = 0;
        for (unsigned rank = 1; rank < size; ++rank) {
            // send next work item if we have it
            bool gotNext = cur_workitem < n_workitems;
            if (gotNext) {
                MPI_Send(regions[cur_workitem++].data, 4, MPI_UNSIGNED, rank, WORKTAG, MPI_COMM_WORLD);
                active_procs++;
            }
            else {
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
            unsigned w   = roi.right - roi.left + 1;
            unsigned h   = roi.bottom - roi.top + 1;

            // receive the rendered bitmap data
            Color3 *result = NEWARR(Color3, w*h);
            MPI_Recv(result, w*h*sizeof(Color3), MPI_BYTE, src, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // copy the received bitmap into our bigger bitmap
            unsigned i = 0;
            for (unsigned y = roi.top; y <= roi.bottom; ++y)
                for (unsigned x = roi.left  ; x <= roi.right; ++x) {
                    image[y*rs.width+x] = result[i++];
                }


            DELARR(result);

            // send next work item if we have it
            if (cur_workitem < n_workitems)
                MPI_Send(regions[cur_workitem++].data, 4, MPI_UNSIGNED, src, WORKTAG, MPI_COMM_WORLD);
            else {
                MPI_Send(0, 0, MPI_INT, src, DIETAG, MPI_COMM_WORLD);
                active_procs--;
            }
        }
        DELARR(regions);
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
                MPI_Finalize();
                exit(0); // no more messages to process
            }
            else if (status.MPI_TAG == WORKTAG) {
                unsigned w = roi.right -  roi.left + 1;
                unsigned h = roi.bottom - roi.top + 1;
                printf("Process %d rendering [%d x %d]", rank, w, h); roi.print(); printf("\n");

                // do the work
                RenderThread *t = new RenderThreadQt(r, image);
                t->setROI(roi.left, roi.right, roi.top, roi.bottom);
                t->run();

                // copy the result into a temp variable
                Color3 *tiny = NEWARR(Color3, w*h);
                unsigned i = 0;
                for (unsigned y = roi.top;  y <= roi.bottom; ++y)
                    for (unsigned x = roi.left; x <= roi.right; ++x)
                        tiny[i++] = image[y*rs.width+x];
                assert(i==w*h);

                // send the result back
                MPI_Send(roi.data, 4, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
                MPI_Send(tiny, w*h*sizeof(Color3), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
                DELARR(tiny);
            }
            else exit(1);
        }
    }
#endif


#ifdef QTHREAD

    unsigned grid_pixel_size = 250;
    for (unsigned y = 0; y < rs.height; y += grid_pixel_size) {
        for (unsigned x = 0; x < rs.width; x += grid_pixel_size) {
            ROI roi;
            roi.top = y;
            roi.bottom = min(y + grid_pixel_size, rs.height) - 1;
            roi.left = x;
            roi.right = min(x + grid_pixel_size, rs.width) - 1;
            roi.print(); printf("\n");
            RenderThreadQt *thread = new RenderThreadQt(r, image);
            thread->setROI(roi.left, roi.right, roi.top, roi.bottom);
            thread->setAutoDelete(true);
            QThreadPool::globalInstance()->start(thread);
        }
    }
    QThreadPool::globalInstance()->waitForDone();

#endif

#ifdef NO_MT
    RenderThreadQt *thread = new RenderThreadQt(r, image);
    thread->run();
#endif

#ifdef USE_QIMAGE
    QImage qi(rs.width, rs.height, QImage::Format_ARGB32);
    for(unsigned y = 0, i = 0; y < rs.height; ++y)
        for(unsigned x = 0; x < rs.width; ++x, ++i)
            qi.setPixel(x, y, image[i].toInt32());
    qi.save("blah.png", "png");
#endif

#ifdef USE_GD
    // Get a gd color using the rgb data.
    gdImagePtr img = gdImageCreateTrueColor(rs.width, rs.height);
    for(unsigned y = 0, i = 0; y < rs.height; ++y) {
        for(unsigned x = 0; x < rs.width; ++x, ++i) {
            real_t R = MIN(image[i].r, 1), G = MIN(image[i].g, 1), B = MIN(image[i].b, 1);
            int gd_color = gdImageColorAllocate(img, R*255, G*255, B*255);
            gdImageSetPixel(img, x, y, gd_color);
        }
    }
    FILE* fd = fopen("blah.png", "wb");
    gdImagePng(img, fd);  // Save the gd image to a png file.
    gdImageDestroy(img);  // Release resources associated with the temporary in-memory bitmap.
    fclose(fd);
    fd = NULL;
#endif

    cout << "\nFile Written." << endl;
    //system("start i_view32 blah.png");
    system("eog blah.png");
    delete   s;
    delete   r;
    delete[] image;
    return 0;
}
