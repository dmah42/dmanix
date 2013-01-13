all: initrd_build

clean:
	rm -f initrd_build

initrd_build: initrd_build.cc
	g++ $< -o $@ -O3 -Wall -Werror -Wextra
