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
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -I/usr/local/Fotech/include -I/usr/local/cernroot/include -I/usr/local/dislin -I/home/fotech/cuda-workspace/Grapher/src -I/opt/local/include/ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


