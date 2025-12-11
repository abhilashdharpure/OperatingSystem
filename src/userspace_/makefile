USER_OBJS = \
  user/init.o \
  user/lib/write.o \
  user/lib/syscall.o

user.bin: $(USER_OBJS)
	ld -T user/link.ld -o user.bin $(USER_OBJS)
