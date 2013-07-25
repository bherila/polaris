// example1.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <limits.h>

#include "scenes.cuh"
#include "defs.cuh"

#define MAX_DEPTH 10
 

    
__global__ void render_block(Scene *scene, RenderSettings settings, float3 *a, int N, int startidx, int samples) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        float y = ((idx + startidx) / settings.width);
        float x = ((idx + startidx) - y * settings.width) / (float) settings.width;
        y /= (float) settings.height;
        a[idx].x = a[idx].y = a[idx].z = 0.0f;
        PathVertex path[MAX_DEPTH];
        SurfacePoint intersectP;
        Ray world_ray;
        for (unsigned i = 0; i < samples; ++i) {
            calc_world_ray(scene, x + urand(threadIdx.x) / (float) settings.width,
                    y + urand(threadIdx.x) / (float) settings.height, &world_ray);

            if (calc_nearest_intersection(scene, &world_ray, &intersectP)) {

                int num = generate_path(scene, &world_ray, threadIdx.x, &path[0], MAX_DEPTH);
                evaluate_path(scene, threadIdx.x, &path[0], num, &a[idx]);
                
                //                calc_direct_illumination(&intersectP, scene,
                //                        urand(threadIdx.x), urand(threadIdx.x), &a[idx]);
                
      //a[idx].x = a[idx].z = 1.0f;
   // a[idx].x = a[idx].y = 0.0f;
            }
        }

        vec3_scalar_mult(&a[idx], 1.0f / (float) samples, &a[idx]);
  
    }
   
}


void setGPU(int gpuNo) {
	CUDA_SAFE_CALL(cudaSetDevice(gpuNo + 1));
}

void render(const RenderSettings *settings, float3 *host_image, ROI roi, int gpuNo) {
  /* unsigned z =roi.startIdx;
   for(;z < roi.startIdx + roi.length;++z) {
   		host_image[z].x = host_image[z].y = 1.0f;
   		
   }*/
   // memset(&host_image[roi.startIdx], 10.0f, sizeof(float3) * roi.length);
//return;
    float3 *device_image, *host_image_tmp;

    Scene *scene = generate_diffuse_spheres(*settings);

    int N = 900000 / settings->samples; //avoid the lame timeout
    if(N > roi.length) N = roi.length; 
//    const size_t host_size = settings->width * settings->height * sizeof (float3);
    const size_t host_size = roi.length * sizeof (float3);
    const size_t device_size = sizeof (float3) * N; //can't allocate too much at once

    Scene *device_scene;
   
    const int num_iterations = host_size / device_size; //really will need to do one more iteration for remainder
    int block_size = 256;
    int n_blocks = N / block_size + (N % block_size == 0 ? 0 : 1);

    //allocate on host & device
   
    cudaEvent_t ev_1, ev_2;
    cudaEventCreate(&ev_1);
    cudaEventCreate(&ev_2);
    cudaEventRecord(ev_1, 0);
   
    host_image_tmp = (float3 *) malloc(device_size);
    cudaMalloc((void **) & device_image, device_size);
    cudaMalloc((void **) & device_scene, sizeof (Scene));
    CUDA_SAFE_CALL(cudaMemcpy(device_scene, scene, sizeof (Scene), cudaMemcpyHostToDevice));

    unsigned i = 0;
    for (; i <= num_iterations; ++i) {
        CUDA_SAFE_CALL(cudaMemcpy(device_image, host_image_tmp, device_size, cudaMemcpyHostToDevice));
        render_block <<< n_blocks, block_size >>> (device_scene, *settings, device_image, N, N*i+ roi.startIdx , settings->samples); //oh snap
        cudaThreadSynchronize();
        CUDA_SAFE_CALL(cudaMemcpy(host_image_tmp, device_image, device_size, cudaMemcpyDeviceToHost));
        if(i == num_iterations) { //if were on the last block 
            memcpy(&host_image[N * i + roi.startIdx], host_image_tmp, sizeof(float3) * (roi.length % N));
        }
        else {
            memcpy(&host_image[N * i + roi.startIdx], host_image_tmp, device_size);
        }
    }
    cudaEventRecord(ev_2, 0);
    cudaEventQuery(ev_1);
    cudaEventQuery(ev_2);
    cudaEventSynchronize(ev_1);
    cudaEventSynchronize(ev_2);
    float time_e;
    cudaEventElapsedTime(&time_e, ev_1, ev_2);
    printf("\033[01mGPU %d \t \033[0mTime taken: %f ms", gpuNo, time_e);
    printf(", Last error: %s\n", cudaGetErrorString(cudaGetLastError()));

    free(host_image_tmp);
    cudaFree(device_image);
//    CUDA_SAFE_CALL(cudaFree(&device_scene->mObjects));
//    CUDA_SAFE_CALL(cudaFree(&device_scene->mEmitters));
    cudaFree(device_scene);
    
    releaseScene(scene);
   // cudaThreadExit();
}



void DisplayProperties(cudaDeviceProp* pDeviceProp) {
    if (!pDeviceProp) return;
    printf("\n\033[00mDevice Name\t\t\t\t - %s \033[0m ", pDeviceProp->name);
   /* printf("\n--------------------------------------------------------------------------");
    printf("\nTotal Global Memory\t\t\t - %d KB", (int)pDeviceProp->totalGlobalMem / 1024);
    printf("\nShared memory available per block \t - %d KB", (int)pDeviceProp->sharedMemPerBlock / 1024);
    printf("\nNumber of registers per thread block \t - %d", pDeviceProp->regsPerBlock);
    printf("\nWarp size in threads \t\t\t - %d", pDeviceProp->warpSize);
    printf("\nMemory Pitch \t\t\t\t - %d bytes", (int)pDeviceProp->memPitch);
    printf("\nMaximum threads per block \t\t - %d", pDeviceProp->maxThreadsPerBlock);
    printf("\nMaximum Thread Dimension (block) \t - %d %d %d", pDeviceProp->maxThreadsDim[0], pDeviceProp->maxThreadsDim[1], pDeviceProp->maxThreadsDim[2]);
    printf("\nMaximum Thread Dimension (grid) \t - %d %d %d", pDeviceProp->maxGridSize[0], pDeviceProp->maxGridSize[1], pDeviceProp->maxGridSize[2]);
    printf("\nTotal constant memory \t\t\t - %d bytes", (int)pDeviceProp->totalConstMem);
    printf("\nCUDA ver \t\t\t\t - %d.%d", pDeviceProp->major, pDeviceProp->minor);
    printf("\nClock rate \t\t\t\t - %d KHz", pDeviceProp->clockRate);
    printf("\nTexture Alignment \t\t\t - %d bytes", (int)pDeviceProp->textureAlignment);
    printf("\nDevice Overlap \t\t\t\t - %s", pDeviceProp-> deviceOverlap ? "Allowed" : "Not Allowed");
    printf("\nNumber of Multi processors \t\t - %d\n\n", pDeviceProp->multiProcessorCount);*/
}

void print_device_properties() {
	cudaDeviceProp deviceProp;
	int nDevCount = 0;

	cudaGetDeviceCount( &nDevCount );
	printf( "\033[34;01mTotal Number of GPUs found: %d\033[0m", nDevCount );
	printf("\n--------------------------------------------------------------------------");
	for (int nDeviceIdx = 0; nDeviceIdx < nDevCount; ++nDeviceIdx )
	{
		memset( &deviceProp, 0, sizeof(deviceProp));
		if( cudaSuccess == cudaGetDeviceProperties(&deviceProp, nDeviceIdx))
			DisplayProperties( &deviceProp );
		else
			printf( "\n%s", cudaGetErrorString(cudaGetLastError()));
	}
}
