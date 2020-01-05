main:
	gcc client.c client.h -o client -g -Wextra -Wall -Werror

clean:
	rm -rf client
