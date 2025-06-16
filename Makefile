API_DIR = api

SOM_BOARD_DIR = som
SOM_BOARD_SRC = main.c $(SOM_BOARD_DIR)/som_board.c
SOM_BOARD_OBJ = main.o som_board.o
SOM_TARGET = som_board_test

JIG_BOARD_DIR = jig
JIG_BOARD_SRC = main.c $(JIG_BOARD_DIR)/jig_board.c $(API_DIR)/menu.c $(JIG_BOARD_DIR)/gpioctl.c
JIG_BOARD_OBJ = main.o jig_board.o menu.o gpioctl.o
JIG_TARGET = jig_board_test

all: som_board jig_board

test:
	$(CC) -o test main.c -I./include

.obj_som_board: $(SOM_BOARD_SRC)
	$(CC) -c $^ -I./include

som_board: .obj_som_board
	$(CC) -o $(SOM_TARGET) $(SOM_BOARD_OBJ)

.obj_jig_board: $(JIG_BOARD_SRC)
	$(CC) -c $^ -I./include

jig_board: .obj_jig_board
	$(CC) -o $(JIG_TARGET) $(JIG_BOARD_OBJ) -lreadline -lncurses -lgpiod

clean:
	- rm *.o

distclean:
	- rm *.o
	- rm -rf $(SOM_TARGET) $(JIG_TARGET) test
