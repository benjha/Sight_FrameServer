################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../communications/asio/impl/src.cpp 

OBJS += \
./communications/asio/impl/src.o 

CPP_DEPS += \
./communications/asio/impl/src.d 


# Each subdirectory must supply rules for building sources it contributes
communications/asio/impl/%.o: ../communications/asio/impl/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../communications -I../header -I../../../Programs/include -O3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


