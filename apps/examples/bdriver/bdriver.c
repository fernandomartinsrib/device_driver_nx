#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <nuttx/config.h>
#include <fcntl.h>

#include <errno.h> 
#include <arch/board/inc/hw_memmap.h>
#include <arch/board/driverlib/debug.h>
#include <arch/board/driverlib/gpio.h>
#include <arch/board/driverlib/sysctl.h>


#ifdef CONFIG_BUILD_KERNEL
int main(int argc, char *argv[])
#else
int bdriver_main(int argc, FAR char *argv[])
#endif
{
    printf("Board driver works\n");

    
    //const int fd = open("/dev/usb0",  O_CREAT | O_RDONLY, 0);
    int ret;
    uint8_t sample = true;

    int fd = open("/dev/usb0", O_RDWR);

    
    if(fd < 0){
        printf("Error to open /dev/ttyACM0");
        return 0;
    }

    // int fd = open("/dev/usb0", O_RDONLY);
    // unsigned char buf2;
    // ssize_t bytes_read;
    // printf("\nfd: %d", fd);
    // while(1){
    //     bytes_read = read(fd, &buf2, 1);
    //     if(bytes_read == -1){
    //         //printf("\n%s\n", strerror(errno));
    //     }

    //     if(bytes_read != -1){
    //         printf("\nBytes_read: %d", (int)bytes_read);
    //         printf("\nCharacter: %c\n", (char)buf2);
    //     }
    //     //usleep(300000);
    // }

    // close(fd);

    
    unsigned char buf[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
    size_t nbytes;
    ssize_t bytes_written;

    nbytes = strlen(buf);
    bytes_written = write(fd,&buf,nbytes);


    unsigned char buf2;

    ssize_t bytes_read;

    bytes_read = read(fd, &buf2, 1);

    //buf[1] = "\0";
    printf("\nBuf2 len: %d\n", (int)bytes_read);
    printf("\nBuf2: %c\n", buf2);

    int test = ioctl(fd, 1, 0);

    return 0;
}
