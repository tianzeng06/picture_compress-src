CC=g++
CFLAGES=-Wall
EXEC=main
OBJS=main.o compress.o
.PHONY:clean

$(EXEC):$(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(CFLAGES)

clean:
	rm -rf $(EXEC)
	rm -rf $(OBJS)
