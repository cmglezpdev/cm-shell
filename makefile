dev:
	gcc ./src/main.c ./src/builtin.c ./src/parser.c ./src/execute.c ./src/utils.c ./src/list.c -o shell && ./shell