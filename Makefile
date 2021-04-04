CC=gcc
GCCFLAGS = -g -Wall -std=c99
GCCLIBS = -lrt -lpthread
EXT_FILES =  resourcesADT.c errorHandling.c
APP = app view slave


all: $(APP)

$(APP): %: %.c
	$(CC) $(GCCFLAGS) $(EXT_FILES) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(APP)