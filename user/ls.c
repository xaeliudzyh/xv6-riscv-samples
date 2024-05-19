#include "../kernel/param.h"
#include "../kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path, int include_dir)
{
  static char buf[DIRSIZ+1];
  char *p;
  int n = 0; // количество добавленных символов
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  // перемещаем имя файла в буфер
  if (include_dir)
  {
      memmove(buf, "./", 2); // добавляем префикс "./" если include_dir установлен
      n = 2; // увеличиваем add на 2 для учета добавленных символов
  }
  memmove(buf + n, p, strlen(p)); // перемещаем имя файла в буфер с учетом добавленных символов
  memset(buf+strlen(p) + n, ' ', DIRSIZ-strlen(p) - n); // заполняем оставшееся место пробелами
  // если были добавлены символы, добавляем null-терминатор
  if (n) buf[n + strlen(p)] = 0;
  return buf; // возвращаем буфер
}

void
ls(char *path)
{
  char buf[512], *p; // буфер для хранения цели символической ссылки
  int fd; // длина цели символической ссылки
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_NOFOLLOW)) < 0) // открываем файл с флагом O_NOFOLLOW, чтобы не разыменовывать символические ссылки
  {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0)
  {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }
  char link_target[MAXPATH]; // массив для хранения цели символической ссылки
  switch(st.type){
  case T_DEVICE:
  case T_SYMLINK:
      // читаем цель символической ссылки
      readlink(path, link_target);
      // выводим информацию о символической ссылке, включая её цель
      printf("%s %s %d %d %l\n", fmtname(path, 0), link_target, st.type, st.ino, st.size);
      break;
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path,0), st.type, st.ino, st.size);
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
          readlink(fmtname(buf, 1), link_target); // читаем цель символической ссылки
          link_target[st.size] = 0; // добавляем null-терминатор в конце строки
          // выводим информацию о символической ссылке, включая её цель
          printf("%s %s %d %d %l\n", fmtname(buf, 0), link_target, st.type, st.ino, st.size);
      }
      else printf("%s %d %d %d\n", fmtname(buf,0), st.type, st.ino, st.size);

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
