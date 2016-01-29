RM := rm -rf

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS = \
./netlinkServer.c \
./netlinkTest.c 

OBJS = \
./netlinkServer.o \
./netlinkTest.o 

C_DEPS = \
./netlinkServer.d \
./netlinkTest.d 

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

INCLUDES = \
-I"/home/capwap/workspace/OpenCP/WTP/APP/NetlinkServer" \
-I"/home/capwap/workspace/OpenCP/WTP"

LIBS := -lnl-3 -lnl-genl-3 -lrt

# Each subdirectory must supply rules for building sources it contributes
%.o: ./%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc $(INCLUDES) -L/usr/lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<" $(LIBS)
	@echo 'Finished building: $<'
	@echo ' '

# All Target
all: NetlinkServer

# Tool invocations
NetlinkServer: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L/usr/lib -o"NetlinkServer" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) NetlinkServer
	-@echo ' '