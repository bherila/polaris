#include "stdafx.h"
#include "renderthread.h"
#include "renderer.h"

#include <iostream>
using namespace std;

void RenderThreadMPI::run() {

}

void RenderThreadMPI::run_master() {

}

void RenderThreadMPI::run_slave() {

}


void RenderThreadQt::run() {
    cout << "Start rendering... " << endl;
    //struct tms tm; // start timer
    m_renderer->render(m_image, this, m_ROI);
    // cout << "Done! Total time: " << tm.tms_utime << endl;

}

void RenderThread::setROI(unsigned left, unsigned right, unsigned top, unsigned bottom) {
    delete m_ROI;
    m_ROI = new ROI();
    m_ROI->top = top;
    m_ROI->bottom = bottom;
    m_ROI->left = left;
    m_ROI->right = right;
}
