#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

void run_programs(char* path) {
    if (fork() == 0) {
        execl(path, "greet", (char*)0);
    }
    int return_code;
    wait(&return_code);
    std::cout << "child return code is " << return_code << std::endl;
}

void create_pipe() {
    /* 
     * Create a pipe and get two file descriptors
     *  p[0]: output side for read
     *  p[1]: input side for write
     */
    int p[2];
    pipe(p);
    std::cout << p[0] << ", " << p[1] << std::endl;

    /* input to the pipe */
    write(p[1], "hello", 5);

    /* get data from the pipe */
    char buf[10];
    read(p[0], buf, 10);
    std::cout << buf << std::endl;

    /* clean up the pipe */
    close(p[0]);
    close(p[1]);
}

void parent2child() {
    int p[2];
    pipe(p);
    std::cout << p[0] << ", " << p[1] << std::endl;
    
    if (fork() == 0) {
        char buf[10];
        close(p[1]); // must close the input side or the child process will keep waiting for data when read is called as the reference count > 0
        std::cout << "child ready" << std::endl;

        /*
         * The behavior of read:
         * 1. when there is no data
         *    a. if the reference count of the input side file descriptor is zero, it returns 0 as there is no more data
         *    b. if the reference count of the input side file descriptor is larger than zero, it waits for data from the input side and blocks
         * 2. when there is data, it reads the actually existing bytes of data, with the maximum amount of the specified number
         */
        while (read(p[0], buf, 15)) {
            std::cout << "read from pipe: " << buf << std::endl;
        }

        std::cout << "child done" << std::endl;
    } else {
        close(p[0]);
        write(p[1], "hello my child!", 15);
        write(p[1], "goodbye, child.", 15);
        // close(p[1]); when this is closed, the child finishes reading and proceeds to end
        sleep(2);
    }
}

int main(int argc, char** argv) {
    if (strcmp(argv[0], "greet") == 0) {
        std::cout << "executing program under the name " << argv[0] << std::endl;
    } else {
        if (argc > 1) {
            run_programs(argv[1]);
        } else {
            create_pipe();
            parent2child();
        }
    }
}
