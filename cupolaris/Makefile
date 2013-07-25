clean:
	@rm -rf *~
	@rm -rf *.o
	@rm -rf *cupolaris 
build: clean
	mpicc -c -I/old-teramite-home/timwar/NVIDIA_CUDA_SDK/common/inc/ -I/old-teramite-usrlocal/cuda/include/ -lcudart -lcuda -lcutil -L/old-teramite-usrlocal/cuda/lib/ -L/old-teramite-home/timwar/NVIDIA_CUDA_SDK/lib/ main.cpp; \
	nvcc --host-compilation=c -I/old-teramite-home/timwar/NVIDIA_CUDA_SDK/common/inc/ -Xcompiler "-w" -L/usr/lib64/openmpi/1.2.3-gcc/ -lmpi -lmpi_cxx -lmca_common_sm kernels.cu main.o -o cupolaris
run: build
	mpirun -np 2 /home/brown110/bherila/cupolaris/cupolaris 360 240 100

scale: build
	mpirun -np 6 /home/brown110/bherila/cupolaris/cupolaris 360 240 90; \
	mpirun -np 6 /home/brown110/bherila/cupolaris/cupolaris 360 240 80; \
	mpirun -np 6 /home/brown110/bherila/cupolaris/cupolaris 360 240 70; \
	mpirun -np 6 /home/brown110/bherila/cupolaris/cupolaris 360 240 60; \
	mpirun -np 5 /home/brown110/bherila/cupolaris/cupolaris 360 240 90; \
	mpirun -np 5 /home/brown110/bherila/cupolaris/cupolaris 360 240 80; \
	mpirun -np 5 /home/brown110/bherila/cupolaris/cupolaris 360 240 70; \
	mpirun -np 5 /home/brown110/bherila/cupolaris/cupolaris 360 240 60; \
	mpirun -np 4 /home/brown110/bherila/cupolaris/cupolaris 360 240 90; \
	mpirun -np 4 /home/brown110/bherila/cupolaris/cupolaris 360 240 80; \
	mpirun -np 4 /home/brown110/bherila/cupolaris/cupolaris 360 240 70; \
	mpirun -np 4 /home/brown110/bherila/cupolaris/cupolaris 360 240 60; \
	mpirun -np 3 /home/brown110/bherila/cupolaris/cupolaris 360 240 90; \
	mpirun -np 3 /home/brown110/bherila/cupolaris/cupolaris 360 240 80; \
	mpirun -np 3 /home/brown110/bherila/cupolaris/cupolaris 360 240 70; \
	mpirun -np 3 /home/brown110/bherila/cupolaris/cupolaris 360 240 60; \
	mpirun -np 2 /home/brown110/bherila/cupolaris/cupolaris 360 240 90; \
	mpirun -np 2 /home/brown110/bherila/cupolaris/cupolaris 360 240 80; \
	mpirun -np 2 /home/brown110/bherila/cupolaris/cupolaris 360 240 70; \
	mpirun -np 2 /home/brown110/bherila/cupolaris/cupolaris 360 240 60 
	
