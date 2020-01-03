# Total time taken by scheduler for  
## MLFQ:- 
3454
## RR:- 
3420
## FCFS:- 
3439
## PBS:- 
3461
# Get pinfo report for every child in MLFQ:-
Child is created order wise. It is best algo.Wait time for every child is almost 
same because of demotion of every process after certain time value. 
## IN ORDER OF ENDTIME :-
### CHILD 9
```
Process id : 17
runtime : 475 ticks
num_run : 1414
curr_q : 4
ticks : 1 2 4 8 458
Process with pid=17 wtime=2908 rtime=475
```
### CHILD 0 
```
Process ID :  8
runtime : 493 ticks
num_run : 1433
curr_q : 4
ticks : 1 2 4 8 476
Process with pid=8 wtime=2914 rtime=493
```
### CHILD 4 
```
Process ID :  12
runtime : 486 ticks
num_run : 1425
curr_q : 4
ticks : 1 2 4 8 469
Process with pid=12 wtime=2941 rtime=486
```
### CHILD 2 
```
Process ID : 10
runtime : 497 ticks
num_run : 1437
curr_q : 4
ticks : 1 2 4 8 480
Process with pid=10 wtime=2967 rtime=497
```
### CHILD 1 
```
Process ID :  9
runtime : 523 ticks
num_run : 1463
curr_q : 4
ticks : 1 2 4 8 506
Process with pid=9 wtime=2994 rtime=523
```
### CHILD 3 
```
Process ID :  11
runtime : 520 ticks
num_run : 1458
curr_q : 4
ticks : 1 2 4 8 502
pid=1
Process with pid=11 wtime=3001 rtime=520
```
### CHILD 5
```
Process ID :  13
runtime : 521 ticks
num_run : 1459
curr_q : 4
ticks : 1 2 4 8 503
Process with pid=13 wtime=3009 rtime=521
```
### CHILD 7 
```
Process ID :  15
runtime : 517 ticks
num_run : 1453
curr_q : 4
ticks : 1 2 4 8 497
Process with pid=15 wtime=3018 rtime=517
```
### CHILD 8 
```
Process ID :  16
runtime : 530 ticks
num_run : 1468
curr_q : 4
ticks : 1 2 4 8 512
pid=1
Process with pid=16 wtime=3019 rtime=530
```
### CHILD 6 

```
Process ID :  14
runtime : 540 ticks
num_run : 1480
curr_q : 4
ticks : 1 2 4 8 525
Process with pid=14 wtime=3011 rtime=544

```

# Waittime and runtime report for every child in PBS:-
Child is created order wise with first created having more priority. Child with
low priority has low waittime. It is second worst scheduling algorithm as it is 
almost equal to FCFS because it is almost same with current priority order.

## IN ORDER OF ENDTIME :-
### CHILD 0 :- 
```
Child with pid=13 wtime=996 rtime=479
```
### CHILD 1 :- 
```
Child with pid=14 wtime=1006 rtime=483
```
### CHILD 2 :- 
```
Child with pid=12 wtime=1467 rtime=488
```
### CHILD 3 :- 
```
Child with pid=11 wtime=1478 rtime=494
```
### CHILD 4 :- 
```
Child with pid=10 wtime=1885 rtime=566
```
### CHILD 5 :- 
```
Child with pid=9 wtime=1919 rtime=543
```
### CHILD 6 :- 
```
Child with pid=8 wtime=2401 rtime=533
```
### CHILD 7 :- 
```
Child with pid=7 wtime=2395 rtime=549
```
### CHILD 8 :- 
```
Child with pid=6 wtime=2936 rtime=478
```
### CHILD 9 :- 
```
Child with pid=5 wtime=2974 rtime=487
```

# Waittime and runtime report for every child in FCFS:-
Child is created order wise.Child created first has low waittime. It is worst 
scheduling algorithm.

## IN ORDER OF ENDTIME :-
### CHILD 0:- 
```
Child with pid=6 wtime=1000 rtime=477
```
### CHILD 1 
```
Child with pid=5 wtime=1000 rtime=482
```
### CHILD 2 
```
Child with pid=7 wtime=1481 rtime=478
```
### CHILD 3 
```
Child with pid=8 wtime=1482 rtime=486
```
### CHILD 4 
```
Child with pid=9 wtime=1956 rtime=481
```
### CHILD 5 
```
Child with pid=10 wtime=1966 rtime=487
```
### CHILD 6
```
Child with pid=11 wtime=2437 rtime=480
```
### CHILD 7 
```
Child with pid=12 wtime=2454 rtime=485
```
### CHILD 8 
```
Child with pid=13 wtime=2917 rtime=480
```        
### CHILD 9
```
Child with pid=14 wtime=2947 rtime=483
```


# Waittime and runtime report for every child in Round Robin:-
Child is created order wise.Every child has almost same waittime. It is second 
best scheduling algorithm.

## IN ORDER OF ENDTIME :-
### CHILD 0 :-
 ```
 Child with pid=6 wtime=999 rtime=477
```
### CHILD 1 :-
 ```
 Child with pid=5 wtime=1001 rtime=483
 ```
### CHILD 2 :-
 ```
 Child with pid=7 wtime=1479 rtime=477
 ```
### CHILD 3 :-
 ```
 Child with pid=8 wtime=1487 rtime=490
 ```
### CHILD 4 :-
 ```Child with pid=9 wtime=1956 rtime=493
 ```
### CHILD 5 :-
 ```
 Child with pid=10 wtime=1977 rtime=474
 ```
### CHILD 6 :-
 ```
 Child with pid=11 wtime=2448 rtime=479
 ```
### CHILD 7 :-
 ```
 Child with pid=12 wtime=2451 rtime=486
 ```
### CHILD 8 :-
 ```
 Child with pid=13 wtime=2927 rtime=486
 ```
### CHILD 9 :-
 ```
 Child with pid=14 wtime=2937 rtime=477
 ```