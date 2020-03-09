#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "fibdrv.h"
#include "record.h"

#define FIB_DEV "/dev/fibonacci"

int test_run(int devfd, char *filename, int len)
{
    unsigned long long sz;
    unsigned long long fibnum[2];
    size_t nrun = 5000;
    record_init();
    for (size_t k = 0; k < nrun; k++) {
        printf("(%zu/%zu)\n", k, nrun);
        for (int i = 0; i <= len; i++) {
            lseek(devfd, i, SEEK_SET);
            sz = read(devfd, fibnum, 1);
            ;
            record(i, sz);
        }
    }
    FILE *fp = fopen(filename, "w");
    record_show(fp);
    fclose(fp);
}

int main()
{
    long long sz;
    unsigned long long fib[2];

    int offset =
        MAX_LENGTH; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    sz = write(fd, "d", 2);
    if (sz == -1) {
        perror("write");
        exit(-1);
    }

    for (int i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        read(fd, fib, 1);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "0x%llx%016llx.\n",
               i, fib[0], fib[1]);
    }

    for (int i = offset; i >= 0; i--) {
        lseek(fd, i, SEEK_SET);
        read(fd, fib, 1);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "0x%llx%016llx.\n",
               i, fib[0], fib[1]);
    }

    char buf[10];
    buf[0] = FIB_REG_128;
    sz = write(fd, buf, 1);
    if (sz == -1) {
        perror("write");
        exit(-1);
    }
    test_run(fd, "fib_r.dat", MAX_LENGTH);

    buf[0] = FIB_DOUB_128;
    sz = write(fd, buf, 1);
    if (sz == -1) {
        perror("write");
        exit(-1);
    }
    test_run(fd, "fib_d.dat", MAX_LENGTH);

    buf[0] = FIB_DOUB_LL;
    sz = write(fd, buf, 1);
    if (sz == -1) {
        perror("write");
        exit(-1);
    }
    test_run(fd, "fib_d_ll.dat", 92);

    buf[0] = FIB_REG_LL;
    sz = write(fd, buf, 1);
    if (sz == -1) {
        perror("write");
        exit(-1);
    }
    test_run(fd, "fib_r_ll.dat", 92);

    close(fd);
    return 0;
}
