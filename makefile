VERSION  = 1.0.0

SOURCE   = $(wildcard ./adapter/source/posix/*.c ./core/source/*.c ./*.c)
OBJECT   = $(patsubst %.c, %.o, $(SOURCE))

INCLUDE  = -I ./adapter/include
INCLUDE += -I ./core/include

TARGET  = tp
CC      = gcc
CFLAGS  = -Wall -g

OUTPUT  = output

$(TARGET): $(OBJECT)
	@mkdir -p $(OUTPUT)
	$(CC) $^ $(CFLAGS) -o $(OUTPUT)/$(TARGET)_$(VERSION) -lpthread

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	@rm -rf $(OBJECT) $(OUTPUT)