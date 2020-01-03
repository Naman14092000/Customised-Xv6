// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "fcntl.h"
// int main(int argc, char *argv[])
// {
//     int value, pid=0;
//     //   char *type;

//     //   if (argc < 3) {
//     //   printf(2, "Usage: nice [type: -p: priority, -t: tickets] [pid] [priority or tickets]\n" );
//     //   exit();
//     //   }
//     //   type = argv[1];
//     //   pid = atoi(argv[2]);
//     value = atoi(argv[1]);

//     //   if(strcmp(type, "-p") == 0)
//     //   {
//     // #ifdef SML
//     //   if (value < 1 || value > 3) {
//     //   printf(2, "Invalid priority (1-3)!\n" );
//     //   exit();
//     //   }
//     // #else
//     if (value < 1 || value > 100)
//     {
//         printf(2, "Invalid priority (1-20)!\n");
//         exit();
//     }
//     // #endif

//     chpr(pid, value);

//     //   }
//     //   else if(strcmp(type, "-t") == 0)
//     //   {
//     //     if (value < 0 || value > 100) {
//     //         printf(2, "Invalid tickets (0-100)!\n" );
//     //         exit();
//     //     }
//     //     chtickets(pid, value);
//     //   }
//     //   else
//     //   {
//     // printf(2, "Usage: nice [type: -p: priority, -t: tickets] [pid] [priority or tickets]\n" );
//     // exit();
//     //   }

//     exit();
// }
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main (int argc,char *argv[])
{
  if(fork() == 0){
    int pid1, pid2;
    int status1, a1, b1;
    int status2, a2, b2;
    pid1 = fork();
    if(pid1 == 0){
        // int pid=0;
      chpr(45);
      sleep(20);
      exec(argv[1], argv);
      printf(1, "exec %s failed\n", argv[1]);
    }else{
      pid2 = fork();
      if(pid2 == 0){
        //   int pid=0;
        chpr(75);
        sleep(20);
        exec(argv[1], argv);
        printf(1, "exec %s failed\n", argv[1]);
      }else{
        status1 = waitx(&a1, &b1);
        status2 = waitx(&a2, &b2);
        printf(1, "Wait Time = %d\n Run Time = %d with PID = %d \n", a1, b1, status1);
        printf(1, "Wait Time = %d\n Run Time = %d with PID = %d \n", a2, b2, status2);
        printf(1, "Child Priority 45 : %d, Child Priority 75 : %d\n", pid1, pid2);
      }
    }
  }else{
    int a,b,c;
    chpr(1);
    a = waitx(&b,&c);
    printf(1, "PID: %d, WTIME: %d, RTIME: %d\n", a, b, c);
  }
  exit();
}
