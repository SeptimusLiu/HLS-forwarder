LIBS = -lm
OBJS = main.o http_server.o http_request_util.o utils.o queue.o
main: ${OBJS}
    gcc -o main ${OBJS} ${LIBS}
clean:
    rm -f main ${OBJS}