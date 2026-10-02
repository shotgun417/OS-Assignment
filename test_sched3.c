#include "types.h"
#include "stat.h"
#include "user.h"

#define DURATION 300  // Ticks 수
#define NWORKER 3

// CPU를 점유하며 실행되는 워커 프로세스
void worker(int nice, int deadline, int fd)
{
  int count = 0, last = -1, now;

  setnice(getpid(), nice);

  while ((now = uptime()) < deadline) {
    if (now != last) {
      count++;
      last = now;
    }
  }

  write(fd, &count, sizeof(count));
  exit();
}

// 지정된 nice 값을 가지는 NWORKER개의 워커를 DURATION 동안 실행 후 실행 tick 수 수집
void run_workers(int *nice, int *count)
{
  int fd[NWORKER][2], deadline, i;

  deadline = uptime() + DURATION;

  for (i = 0; i < NWORKER; i++) {
    pipe(fd[i]);
    if (fork() == 0) {
      worker(nice[i], deadline, fd[i][1]);
    }
  }

  for (i = 0; i < NWORKER; i++) {
    read(fd[i][0], &count[i], sizeof(count[i]));
    close(fd[i][0]);
    close(fd[i][1]);
  }

  for (i = 0; i < NWORKER; i++) {
    wait();
  }
}

// 결과를 퍼센티지(소수점 한 자릿수) 형태로 출력
void report(int id, int *nice, int *count, int ok)
{
  int i, sum = 0, share;

  for (i = 0; i < NWORKER; i++) {
    sum += count[i];
  }

  printf(1, "case %d. nice %d %d %d -> cpu ", id, nice[0], nice[1], nice[2]);

  for (i = 0; i < NWORKER; i++) {
    share = sum ? count[i] * 1000 / sum : 0;
    printf(1, "%d.%d%% ", share / 10, share % 10);
  }

  printf(1, ": %s\n", ok ? "OK" : "WRONG");
}

int main(int argc, char **argv)
{
  int nice[NWORKER], count[NWORKER], sum, ok, i;

  // 부모 프로세스가 워커 생성 및 결과 수집을 위해 최우선순위(nice 0)로 설정
  setnice(getpid(), 0);

  // Case 1: nice 10 프로세스는 기아 상태(Starvation)에 빠지지 않고 적은 비율로 실행되어야 함
  nice[0] = 0; nice[1] = 0; nice[2] = 10;
  run_workers(nice, count);
  ok = count[2] > 0 && count[0] > count[2] && count[1] > count[2];
  report(1, nice, count, ok);

  // Case 2: nice 값이 작을수록 더 많은 CPU 점유율을 가짐
  nice[0] = 1; nice[1] = 5; nice[2] = 9;
  run_workers(nice, count);
  ok = count[2] > 0 && count[0] > count[1] && count[1] > count[2];
  report(2, nice, count, ok);

  // Case 3: 동일한 우선순위인 경우 공평하게 점유율을 나누어 가짐
  nice[0] = 3; nice[1] = 3; nice[2] = 3;
  run_workers(nice, count);
  sum = count[0] + count[1] + count[2];
  ok = sum > 0;
  for (i = 0; i < NWORKER; i++) {
    if (count[i] * NWORKER * 2 < sum || count[i] * NWORKER * 2 > sum * 3) {
      ok = 0;
    }
  }
  report(3, nice, count, ok);

  exit();
}
