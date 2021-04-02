CC=gcc
GCCFLAGS = -g -Wall -std=c99
GCCLIBS = -lrt -lpthread
APP = app view slave


all: $(APP)

$(APP): %: %.c
	@$(CC) $(GCCFLAGS) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(APP)