#include "../kernel/param.h"
#include "../kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p, symlink_target[MAXPATH]; // буфер для хранения цели символической ссылки
  int fd,symlink_target_len; // длина цели символической ссылки
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_NOFOLLOW)) < 0) // открываем файл с флагом O_NOFOLLOW, чтобы не разыменовывать символические ссылки
  {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_DEVICE:
  case T_SYMLINK:
      // попытка чтения цели символической ссылки
      if ((symlink_target_len = readlink(path, symlink_target)) < 0) printf("%s %d %d %l --> unknown target\n", fmtname(path), st.type, st.ino, st.size);
      else
      {
          if (symlink_target_len < MAXPATH)
          {
              symlink_target[symlink_target_len] = '\0'; // добавление нуль-терминатора в конец буфера
              printf("%s %d %d %l --> %s\n", fmtname(path), st.type, st.ino, st.size,
                     symlink_target); // вывод информации о символической ссылке
          }
      }
      break;
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
        // используем lstat для получения информации о символических ссылках без их разыменования
      if(lstat(buf, &st) < 0)
      {
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      if (st.type == T_SYMLINK)
      {
          // показать цель ссылки
          if ((symlink_target_len = readlink(path, symlink_target)) < 0) printf("%s %d %d %l --> unknown target\n", fmtname(path), st.type, st.ino, st.size);
          else
          {
              if (symlink_target_len < MAXPATH) symlink_target[symlink_target_len] = '\0'; // добавление нуль-терминатора
              printf("%s %d %d %l --> %s\n", fmtname(path), st.type, st.ino, st.size, symlink_target); // вывод цели символической ссылки
          }
      }
      else printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);

    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
