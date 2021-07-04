target: final

final: 	Server.c
		gcc Server.c graph.c threadlist.c cache.c -o server -lpthread -Wall -g
		gcc Client.c -o client -Wall -g
clean:
	rm server