CC=gcc

appmake: app.c slave.c
	@$(CC) -g app.c -o app -std=c99
	@$(CC) -g slave.c -o slave -std=c99

.PHONY: clean
clean:
	@rm -f slave app