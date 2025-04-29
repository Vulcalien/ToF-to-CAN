#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool write_all(int fd, const char *data, size_t len)
{
	while (len != 0)
	{
		int r = write(fd, data, len);
		if (r <= 0)
		{
			perror("write");
			return false;
		}

		data += r;
		len -= r;
	}

	return true;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s /dev/ttyXXX\n", argv[0]);
		return EXIT_SUCCESS;
	}

	int uart = open(argv[1], O_RDWR | O_NOCTTY);
	if (uart < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	while (true)
	{
		const char *text = "Hello, world!\r\n";

		write_all(uart, text, strlen(text));
		sleep(1);
	}

	close(uart);
	return EXIT_SUCCESS;
}
