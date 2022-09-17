#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

char *fmtname(char *path) {
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

uint divideIntoBlocks(uint size) {
    return ((size + BSIZE - 1) / BSIZE);
}


void du(char *path, int blocksFlag, uint threshold) {
  char buf[BSIZE], *p;
  int fd;
  int sum = 0;
  struct dirent de;
  struct stat st;
  uint sizeOf;

  if ((fd = open(path, 0)) < 0) {//is this the best place to exit;
    printf(2, "check usage.\n");
    exit();
    return;
  }

  if (fstat(fd, &st) < 0) {
    printf(2, "du: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_FILE:
    if (blocksFlag == 1) {
        sizeOf = divideIntoBlocks(st.size);
    } else {
        sizeOf = st.size;
    }
    sum = sum + sizeOf;
    printf(1, "%d %s\n", sizeOf, path);
    printf(1, "%d %s\n", sum, path);
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path); //revise this area of the code to see where you can eliminate unnecesary ls
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (stat(buf, &st) < 0) {
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      if (st.type == T_FILE && st.size >= threshold) {
          if (blocksFlag == 1) {
              sizeOf = divideIntoBlocks(st.size);
          } else {
              sizeOf = st.size;
          }
          printf(1, "%d %s\n", sizeOf, fmtname(buf));
          sum += sizeOf;
      }
    }
    printf(1, "%d %s\n", sum, fmtname(path));
    break;
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int i;
  int blocksFlag = 0;
  int thresholdFlag = 0;
  uint threshold = 0;

  if (argc < 2) {
    du(".", 0, 0);
    exit();
  }

  int validArguments = 0; //I'm not a fan of how I'm parsing through flags currently
  for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-k") == 0) { 
          if (blocksFlag == 0) {
              blocksFlag = 1;
              validArguments++;
              continue;
          } else {
          printf(1, "check usage.\n");
          exit();
          }
      }
      if (strcmp(argv[i], "-t") == 0) {
          if (thresholdFlag == 0) {
              thresholdFlag = 1;
              threshold = atoi(argv[++i]);
              validArguments += 2;
              continue;
          } else {
          printf(1, "check usage.\n");
          exit();
          }
      }
  }

  if (validArguments + 1 == argc) {
      du(".", blocksFlag, threshold);
  } else {
      for (i = validArguments + 1; i < argc; i++) {
          du(argv[i], blocksFlag, threshold);
  }
  }
  exit();
}