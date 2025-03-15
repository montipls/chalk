all:
	@echo [info] Compiling...
	@gcc -I src/include -L src/lib -O3 -ffast-math -lm -o main.exe main.c engine.c renderer.c -lSDL3 -lSDL3_ttf

run: all
	@echo [info] Running program...
	@echo.
	@./main.exe
	@echo.