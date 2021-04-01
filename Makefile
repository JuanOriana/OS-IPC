CC=gcc

appmake: app.c slave.c
	@$(CC) -g app.c -o app -lrt -lpthread -Wall -std=c99
	@$(CC) -g slave.c -o slave -Wall -std=c99
	@$(CC) -g view.c -o view -lrt -lpthread -Wall -std=c99

.PHONY: clean
clean:
	@rm -f slave app view