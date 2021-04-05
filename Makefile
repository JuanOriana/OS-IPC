CC=gcc
GCCFLAGS = -g -Wall -std=c99 -fsanitize=address
GCCLIBS = -lrt -lpthread
EXT_FILES =  resourcesADT.c errorHandling.c libIPC.c
APP = app view slave


all: $(APP)

$(APP): %: %.c
	@$(CC) $(GCCFLAGS) $(EXT_FILES) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(APP)