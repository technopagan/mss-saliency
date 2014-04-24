################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ImageLoader.cpp \
../src/SaliencyDetector.cpp \
../src/SymmetricSurroundSaliency.cpp 

OBJS += \
./src/ImageLoader.o \
./src/SaliencyDetector.o \
./src/SymmetricSurroundSaliency.o 

CPP_DEPS += \
./src/ImageLoader.d \
./src/SaliencyDetector.d \
./src/SymmetricSurroundSaliency.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -march=native -Wall -c -fmessage-length=0 `Magick++-config --cppflags --cxxflags --ldflags --libs` -v -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


