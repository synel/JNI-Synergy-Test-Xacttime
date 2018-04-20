/*------------------------------------------------------------------------
 * description：  
 * filename - auxi.c 
 *
 * functions list： 
 *        force_open     - create file forcibly..
 *        file_exist     - judge if the file has exist
 *        file_rename    - rename the file
 *        file_delete    - delete the file
 *        file_length    - count file's length
 *        root_dir       - 
 *        efprintf       - print error info
 *        sysreboot      - reboot
 *-----------------------------------------------------------------------*/

/*
 *  辅助（auxiliary）函数库源文件 
 */

/* followings are header files used*/

#include "auxi.h"

/* followings are macro define*/

//#ifdef __cplusplus
//extern "C" {
//#endif

//#ifdef __cplusplus
//}
//#endif

/*--------------------------------------------------------------------------*
函数名字            force_open - 根据指定的路径强制建立文件
函数原型            FILE *force_open(char *path, char *mode)
函数所在头文件      auxi.h
函数的详细描述      path：指向路径的指针；
                    mode：打开方式，与 fopen 相同。 
                    它根据指定的路径以指定方式建立文件。如果指定了并不存
                    在的路径，则先建立相应的目录，最后以指方式建立文件。
                    其作用与 fopen 相同。当使用完毕后，要用 fclose 关闭
                    打开的文件。 
函数返回值          如果文件建立成功返回指向新打开的文件的指针；否则返回
                    NULL。 
*--------------------------------------------------------------------------*/
FILE *force_open(char *path,  char *mode) {

  FILE *fp;
  char *p, *q = (char *)path;

  if(path == NULL || mode == NULL)
    return NULL;

  q = (char *)root_dir(q);

  if((fp = fopen(q, mode)) != NULL)
    return fp;
  else {
    for(p = strchr(q, '/'); p != NULL; p = strchr(p + 1, '/')) {
      if(p != q && *(p - 1) != ':') {
        char ch = *p;
        *p = '\0';       /* 把 full_path 的前半部作成一个字符串 */
            /* 这里是个没有建立的目录，要建立它 */
            /* 当错误代码为 EEXIST 时，说明目录已经存在，不需再建立 */
            /* 如果是其它的错误代码，则返回 NULL。 */
        if(mkdir(q, 0777) != 0 && errno != EEXIST)
          return NULL;
        *p = ch;
      }
    }
  }
  return fopen(q, mode);
} /* force_open */

/*--------------------------------------------------------------------------*
函数名字            file_exist - 判断文件是否存在
函数原型            int file_exist(char *file_name)
函数所在头文件      auxi.h
函数的详细描述      file_name：指向文件名的指针；
                    判断文件是否存在,并且有读写权限。
函数返回值          如果文件存在返回 1, 否则返回 0。 
*--------------------------------------------------------------------------*/
int file_exist(char *file_name) {

  char *p = file_name;
  if(p == NULL)
    return 0;
  p = root_dir(p);
  return access(p, F_OK) == 0;
} /* file_exist */

/*--------------------------------------------------------------------------*
函数名字          file_rename    - 改文件名
函数原型          int file_rename(char *old_file_name, char *new_file_name)
函数所在头文件    auxi.h
函数的详细描述    old_file_name：指向旧文件名的指针；
                  new_file_name：指向新文件名的指针。 
                  改文件名。 
函数返回值        如果成功返回 1, 否则返回 0。 
*--------------------------------------------------------------------------*/
int file_rename(char *old_file_name, char *new_file_name) {

    char *p = old_file_name, *q = new_file_name;
    if(p == NULL || q == NULL)
        return 0;

    p = root_dir(p);
    q = root_dir(q);

    return rename(p, q) == 0;
} /* file_rename */

/*--------------------------------------------------------------------------*
函数名字            file_delete    - 删除文件
函数原型            int file_delete(char *file_name)
函数所在头文件      auxi.h
                    file_name：指向文件名的指针。
                    删除文件。使用 remove 删除普通文件或者目录文件。
函数返回值          如果成功返回 1, 否则返回 0。 
*--------------------------------------------------------------------------*/
int file_delete(char *file_name) {

    char *p = file_name;

    if(p == NULL)
        return 0;

    p = root_dir(p);
    if(chmod(p, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
       == -1) {
      //efprintf(NULL, "chmod %s: %s\n", file_name, strerror(errno));
      return 1;
    }
    if(remove(p) != 0) {
     // efprintf(NULL, "remove %s: %s\n", file_name, strerror(errno));
      return 1;
    }
    return  1;
} /* file_delete */

/*--------------------------------------------------------------------------*
函数名字            file_length    - 计算文件长度
函数原型            long int file_length(char *file_name)
函数所在头文件      auxi.h
                    file_name：指向文件名的指针。
                    计算出给定文件的长度。
函数返回值          如果成功计算出文件的长度，返回它；否则返回 -1。 
*--------------------------------------------------------------------------*/
long int file_length(char *file_name) {

    char *p = root_dir(file_name);
    struct stat statbuf;

    if(stat((char *)p, &statbuf) != 0)
        return -1;
    return statbuf.st_size;
} /* file_length */

/*--------------------------------------------------------------------------*
函数名字            root_dir       - 计算根目录
函数原型            char *root_dir(char *dir)
函数所在头文件      auxi.h
                    dir：指向目录的指针。
                    计算出给定目录的根目录。在这当前的实现中，只是为了配合
                    42 终端通信协议，把 dir 起始的两个表示当前目录的反斜线
                    去掉。它不检查参数的合法性。同时，这个函数把 Windows格
                    式路径转换成 Linux 格式的路径。
函数返回值          把回计算出的根目录的指针。 
*--------------------------------------------------------------------------*/
char *root_dir(char *dir) {

  char *p, *q;

  /* 表明以当前工作目录为根目录 */
  dir =  (dir[0] == '\\' && dir[1] == '\\') ? dir + 2 : dir;
  q = dir;
  while((p = strchr(q, '\\')) != NULL) {
    *p = '/';
    q = p;
  }
  return dir;
} /* root_dir */

/*--------------------------------------------------------------------------*
函数名字            efprintf     - 打印错误信息 
函数原型            int efprintf(FILE *stream, char *fmt, ...);
函数所在头文件      auxi.h
                    打印错误信息.当stream为NULL时，输出到标准错误。
函数返回值          返回实际打印的字节数,出错时返回 -1.
*--------------------------------------------------------------------------*/
int efprintf(FILE *stream, char *fmt, ...) {
  va_list args;
  FILE *s;
  int n;

  s = stream == NULL ? stderr : stream;
  va_start(args, fmt);
  n = vfprintf(s, fmt, args);
  va_end(args);
  return n;
}

/*--------------------------------------------------------------------------*
函数名字            sysreboot            - 重新启动
函数原型            void sysreboot(void)
函数所在头文件      auxi.h
                    重新启动考勤系统.
函数返回值          无.
*--------------------------------------------------------------------------*/
void sysreboot(void) {

  sync();
  reboot(RB_AUTOBOOT);
} /* sysreboot */
