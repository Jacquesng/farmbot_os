#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define INTERVAL 3

struct pstat {
    long unsigned int utime_ticks;
    long int cutime_ticks;
    long unsigned int stime_ticks;
    long int cstime_ticks;
    long unsigned int vsize; // virtual memory size in bytes
    long unsigned int rss; //Resident  Set  Size in bytes
};

int get_usage(const pid_t pid, struct pstat* result) {

    //convert  pid to string
    char pid_s[20];
    snprintf(pid_s, sizeof(pid_s), "%d", pid);

    char stat_filepath[30] = "/proc/"; strncat(stat_filepath, pid_s,
            sizeof(stat_filepath) - strlen(stat_filepath) -1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) -
            strlen(stat_filepath) -1);

    FILE *fpstat = fopen(stat_filepath, "r");
    if (fpstat == NULL) {
        perror("FOPEN ERROR ");
        return -1;
    }

    //read values from /proc/pid/stat
    bzero(result, sizeof(struct pstat));
    long int rss;
    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
                "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",
                &result->utime_ticks, &result->stime_ticks,
                &result->cutime_ticks, &result->cstime_ticks, &result->vsize,
                &rss) == EOF) {
        fclose(fpstat);
        return -1;
    }
    fclose(fpstat);
    result->rss = rss * getpagesize();

    return 0;
}

void calc_cpu_usage_pct(const struct pstat* cur_usage,
                        const struct pstat* last_usage,
                        double* usage)
{
    const long unsigned int pid_diff =
        ( cur_usage->utime_ticks + cur_usage->stime_ticks ) -
        ( last_usage->utime_ticks + last_usage->stime_ticks );

    printf( "delta %lu\r\n", pid_diff );

    *usage = 1/(float)INTERVAL * pid_diff;
}

int main( int argc, char* argv[] )
{
    pstat prev, curr;
    double pct;

    struct tms t;
    times( &t );

    if( argc <= 1 ) {
        printf( "please supply a pid\r\n" ); return 1;
    }

    while( 1 )
    {
        if( get_usage(atoi(argv[1]), &prev) == -1 ) {
            printf("error\r\n" );
        }

        sleep( INTERVAL );

        if( get_usage(atoi(argv[1]), &curr) == -1 ) {
            printf("error\r\n" );
        }

        calc_cpu_usage_pct(&curr, &prev, &pct);

        fprintf(stderr, "%%cpu: %.02f\r\n", pct);
    }
}
