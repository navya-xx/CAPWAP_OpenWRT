RM := rm -rf

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS = \
./ACDataInfoClient.c 

OBJS = \
./ACDataInfoClient.o 

C_DEPS = \
./ACDataInfoClient.d 

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

INCLUDES = \
-I"/home/capwap/workspace/capwap-latest/AC/APP" \
-I"/home/capwap/workspace/capwap-latest/AC"

LIBS := -lrt

# Each subdirectory must supply rules for building sources it contributes
%.o: ./%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc $(INCLUDES) -L/usr/lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<" $(LIBS)
	@echo 'Finished building: $<'
	@echo ' '

# All Target
all: ACDataInfoClient

# Tool invocations
ACDataInfoClient: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L/usr/lib -o"ACDataInfoClient" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) ACDataInfoClient
	-@echo ' '