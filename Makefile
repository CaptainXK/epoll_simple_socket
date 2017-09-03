.PHONY:test,clean,all

all:client server
# 	@$(EXEC) ./server;\
#	@$(EXEC) ./client
	@echo "create done";
server:server.c
	@gcc server.c -o server
client:client.c
	@gcc client.c -o client
clean:
	@rm server client
