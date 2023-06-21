SRC_DIR := src
OBJ_DIR := obj
# all src files
SRC := $(wildcard $(SRC_DIR)/*.c)
# all objects
OBJ_BASE := $(OBJ_DIR)/y.tab.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/response_parse.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/log.o $(OBJ_DIR)/cgi.o
# all binaries
BIN := example liso_server echo_client
# C compiler
CC  := gcc
# C PreProcessor Flag
CPPFLAGS := -Iinclude
# compiler flags
CFLAGS   := -g -Wall
# DEPS = parse.h y.tab.h

default: all
all :clean example liso_server echo_client login

example: $(OBJ_BASE) $(OBJ_DIR)/example.o
	$(CC) $^ -o $@

$(SRC_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l
	flex -o $@ $^

$(SRC_DIR)/y.tab.c: $(SRC_DIR)/parser.y
	yacc -d $^
	mv y.tab.c $@
	mv y.tab.h $(SRC_DIR)/y.tab.h

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

liso_server: $(OBJ_BASE) $(OBJ_DIR)/liso_server.o 
	$(CC) -Werror $^ -o $@

echo_client: $(OBJ_BASE) $(OBJ_DIR)/echo_client.o
	$(CC) -Werror $^ -o $@

login: 
	$(CC) $(SRC_DIR)/login.c -o ./static_site/login.cgi

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ) $(BIN) $(SRC_DIR)/lex.yy.c $(SRC_DIR)/y.tab.*
	$(RM) -r $(OBJ_DIR)
	$(RM) ./static_site/login.cgi

tar:
	$(RM) ../week2.tar
	tar -cvf ../week2.tar *

utils:
	ls -Rl ../..
	cat cp2_checker.py
