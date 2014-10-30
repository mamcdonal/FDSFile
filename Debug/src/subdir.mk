################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/FDSFile.cpp \
../src/FDSHeader.cpp \
../src/main.cpp 

OBJS += \
./src/FDSFile.o \
./src/FDSHeader.o \
./src/main.o 

CPP_DEPS += \
./src/FDSFile.d \
./src/FDSHeader.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-6.5/bin/nvcc -D__GXX_EXPERIMENTAL_CXX0X__ -I/opt/local/include/ -I/usr/local/Fotech/include -I/usr/local/cuda-6.5/targets/x86_64-linux/include -G -g -O0 -Xcompiler -std=c++0x -gencode arch=compute_35,code=sm_35  -odir "src" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-6.5/bin/nvcc -D__GXX_EXPERIMENTAL_CXX0X__ -I/opt/local/include/ -I/usr/local/Fotech/include -I/usr/local/cuda-6.5/targets/x86_64-linux/include -G -g -O0 -Xcompiler -std=c++0x --compile  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


