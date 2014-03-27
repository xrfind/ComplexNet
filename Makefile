#prerequisite: no src/*/*/*.c
#get all source file name in src, include pathname.
#get all obj file name in obj.
#get all directory name in src.
SRC_SRC = $(wildcard src/*.c) $(wildcard src/*/*c)
SRC_DIR = $(sort $(dir $(SRC_SRC)))
SRC_OBJ = $(patsubst %.c, obj/%.o, $(SRC_SRC))

#prerequisite: no main/*/*/*.c
#get all source file name in main, include pathname.
#get all obj file name in obj.
#get all directory name in main.
MAIN_ALL_SRC = $(wildcard main/*.c) $(wildcard main/onion/*.c)
MAIN_ALL_DIR = $(sort $(dir $(MAIN_ALL_SRC)))
MAIN_ALL_OBJ = $(patsubst %.c, obj/%.o, $(MAIN_ALL_SRC))
MAIN_ALL_EXEC = $(subst /,-, $(patsubst main/%.c, %, $(MAIN_ALL_SRC)))


#include dir
INCLUDE_DIR := $(patsubst %, -I%, $(SRC_DIR) $(MAIN_ALL_DIR))

CC := gcc
CFLAG :=  -g -Wall -Wunused 


.PHONY : all clean

all: $(MAIN_ALL_EXEC)

#################################################################
generatenet_T: $(common_objs) obj/main/generatenet_T.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

generatenet_AN: $(common_objs) obj/main/generatenet_AN.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

generatenet_initNet: $(common_objs) obj/main/generatenet_initNet.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

onion_NR: $(common_objs) obj/main/onion_NR.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

onion_3parts_topR: $(common_objs) obj/main/onion_3parts_topR.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

onion_3parts_bestR: $(common_objs) obj/main/onion_3parts_bestR.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

onion_expR : $(common_objs) obj/main/onion_expR.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

onion_corS : $(common_objs) obj/main/onion_corS.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

onion_corR : $(common_objs) obj/main/onion_corR.o
	$(CC) $(CFLAG) -lm $^ -o $@ 
>>>>>>> GenerateNet

#################################################################
MAIN_ONION_SRC = $(wildcard main/onion/*.c)
MAIN_ONION_EXEC = $(subst /,-, $(patsubst main/%.c, %, $(MAIN_ONION_SRC)))
$(MAIN_ONION_EXEC) : onion-% : $(SRC_OBJ) obj/main/onion/%.o
	$(CC) $(CFLAG) -lm $^ -o $@ 

MAIN_SRC = $(wildcard main/*.c) 
MAIN_EXEC = $(basename $(notdir $(MAIN_SRC)))
$(MAIN_EXEC) : % : $(SRC_OBJ) obj/main/%.o
	$(CC) $(CFLAG) -lm $^ -o $@ 
#################################################################



#################################################################
obj/src/%.o : src/%.c src/%.h
	$(CC) $(CFLAG) $(INCLUDE_DIR) -c $< -o $@

obj/main/%.o: main/%.c
	$(CC) $(CFLAG) $(INCLUDE_DIR) -c $< -o $@
#################################################################



#################################################################
clean : 
	$(RM) $(MAIN_ALL_OBJ)
	$(RM) $(SRC_OBJ)
	$(RM) $(MAIN_ALL_EXEC)
#################################################################
