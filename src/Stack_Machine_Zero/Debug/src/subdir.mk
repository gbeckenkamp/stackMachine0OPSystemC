################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Alu.cpp \
../src/DataStack.cpp \
../src/Fetch.cpp \
../src/FileHandler.cpp \
../src/Main.cpp \
../src/Ram.cpp \
../src/ReturnStack.cpp \
../src/Rom.cpp 

OBJS += \
./src/Alu.o \
./src/DataStack.o \
./src/Fetch.o \
./src/FileHandler.o \
./src/Main.o \
./src/Ram.o \
./src/ReturnStack.o \
./src/Rom.o 

CPP_DEPS += \
./src/Alu.d \
./src/DataStack.d \
./src/Fetch.d \
./src/FileHandler.d \
./src/Main.d \
./src/Ram.d \
./src/ReturnStack.d \
./src/Rom.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/systemc231/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


