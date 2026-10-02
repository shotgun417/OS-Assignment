#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv)
{
  int pid;
  int mypid;

  // init 프로세스의 우선순위를 변경
  setnice(1, 10);

  // 현재 프로세스의 우선순위 변경 (주석 처리된 경우 기본 동작 확인)
  // setnice(getpid(), 2);

  // 자식 프로세스 생성
  pid = fork();

  if (pid == 0) {
    // 자식 프로세스 (Child)
    printf(1, "#### State 2 ####\n");
  } else {
    // 부모 프로세스 (Parent)
    printf(1, "#### State 1 ####\n");

    // 자식 프로세스의 우선순위를 변경
    setnice(pid, 10);

    wait();
    printf(1, "#### State 3 ####\n");
  }

  mypid = getpid();
  printf(1, "PID %d is finished\n", mypid);

  exit();
}
