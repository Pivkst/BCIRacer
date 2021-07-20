#OBJS specifies which files to compile as part of the project
OBJS += main.cpp

#CC specifies which compiler we're using
CC = g++

#INCLUDE_PATHS specifies additional include paths
INCLUDE_PATHS += '-I../../C++ resources/SDL2-2.0.9/x86_64-w64-mingw32/include/SDL2'
INCLUDE_PATHS += '-I../../C++ resources/SDL2_mixer-2.0.4/x86_64-w64-mingw32/include/SDL2'
INCLUDE_PATHS += '-I../../C++ resources/SDL2_ttf-2.0.15/x86_64-w64-mingw32/include/SDL2'
INCLUDE_PATHS += '-I../../C++ resources/SDL2_image-2.0.5/x86_64-w64-mingw32/include/SDL2'
INCLUDE_PATHS += '-I../../C++ resources/SDL2_giflib_sa'

#LIBRARY_PATHS specifies additional library paths
LIBRARY_PATHS += '-L../../C++ resources/SDL2-2.0.9/x86_64-w64-mingw32/lib'
LIBRARY_PATHS += '-L../../C++ resources/SDL2_ttf-2.0.15/x86_64-w64-mingw32/lib/'
LIBRARY_PATHS += '-L../../C++ resources/SDL2_mixer-2.0.4/x86_64-w64-mingw32/lib/'
LIBRARY_PATHS += '-L../../C++ resources/SDL2_image-2.0.5/x86_64-w64-mingw32/lib/'

#COMPILER_FLAGS specify the additional compilation options
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
COMPILER_FLAGS = -w -Wl,-subsystem,windows -g

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lws2_32 -lSDL2_ttf -lSDL2_mixer -lSDL2_image

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = debug/BCIGAME

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
