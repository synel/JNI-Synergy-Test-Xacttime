/*------------------------------------------------------------------------
 * description��  
 * filename - auxi.c 
 *
 * functions list�� 
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
 *  ������auxiliary��������Դ�ļ� 
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
��������            force_open - ����ָ����·��ǿ�ƽ����ļ�
����ԭ��            FILE *force_open(char *path, char *mode)
��������ͷ�ļ�      auxi.h
��������ϸ����      path��ָ��·����ָ�룻
                    mode���򿪷�ʽ���� fopen ��ͬ�� 
                    ������ָ����·����ָ����ʽ�����ļ������ָ���˲�����
                    �ڵ�·�������Ƚ�����Ӧ��Ŀ¼�������ָ��ʽ�����ļ���
                    �������� fopen ��ͬ����ʹ����Ϻ�Ҫ�� fclose �ر�
                    �򿪵��ļ��� 
��������ֵ          ����ļ������ɹ�����ָ���´򿪵��ļ���ָ�룻���򷵻�
                    NULL�� 
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
        *p = '\0';       /* �� full_path ��ǰ�벿����һ���ַ��� */
            /* �����Ǹ�û�н�����Ŀ¼��Ҫ������ */
            /* ���������Ϊ EEXIST ʱ��˵��Ŀ¼�Ѿ����ڣ������ٽ��� */
            /* ����������Ĵ�����룬�򷵻� NULL�� */
        if(mkdir(q, 0777) != 0 && errno != EEXIST)
          return NULL;
        *p = ch;
      }
    }
  }
  return fopen(q, mode);
} /* force_open */

/*--------------------------------------------------------------------------*
��������            file_exist - �ж��ļ��Ƿ����
����ԭ��            int file_exist(char *file_name)
��������ͷ�ļ�      auxi.h
��������ϸ����      file_name��ָ���ļ�����ָ�룻
                    �ж��ļ��Ƿ����,�����ж�дȨ�ޡ�
��������ֵ          ����ļ����ڷ��� 1, ���򷵻� 0�� 
*--------------------------------------------------------------------------*/
int file_exist(char *file_name) {

  char *p = file_name;
  if(p == NULL)
    return 0;
  p = root_dir(p);
  return access(p, F_OK) == 0;
} /* file_exist */

/*--------------------------------------------------------------------------*
��������          file_rename    - ���ļ���
����ԭ��          int file_rename(char *old_file_name, char *new_file_name)
��������ͷ�ļ�    auxi.h
��������ϸ����    old_file_name��ָ����ļ�����ָ�룻
                  new_file_name��ָ�����ļ�����ָ�롣 
                  ���ļ����� 
��������ֵ        ����ɹ����� 1, ���򷵻� 0�� 
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
��������            file_delete    - ɾ���ļ�
����ԭ��            int file_delete(char *file_name)
��������ͷ�ļ�      auxi.h
                    file_name��ָ���ļ�����ָ�롣
                    ɾ���ļ���ʹ�� remove ɾ����ͨ�ļ�����Ŀ¼�ļ���
��������ֵ          ����ɹ����� 1, ���򷵻� 0�� 
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
��������            file_length    - �����ļ�����
����ԭ��            long int file_length(char *file_name)
��������ͷ�ļ�      auxi.h
                    file_name��ָ���ļ�����ָ�롣
                    ����������ļ��ĳ��ȡ�
��������ֵ          ����ɹ�������ļ��ĳ��ȣ������������򷵻� -1�� 
*--------------------------------------------------------------------------*/
long int file_length(char *file_name) {

    char *p = root_dir(file_name);
    struct stat statbuf;

    if(stat((char *)p, &statbuf) != 0)
        return -1;
    return statbuf.st_size;
} /* file_length */

/*--------------------------------------------------------------------------*
��������            root_dir       - �����Ŀ¼
����ԭ��            char *root_dir(char *dir)
��������ͷ�ļ�      auxi.h
                    dir��ָ��Ŀ¼��ָ�롣
                    ���������Ŀ¼�ĸ�Ŀ¼�����⵱ǰ��ʵ���У�ֻ��Ϊ�����
                    42 �ն�ͨ��Э�飬�� dir ��ʼ��������ʾ��ǰĿ¼�ķ�б��
                    ȥ���������������ĺϷ��ԡ�ͬʱ����������� Windows��
                    ʽ·��ת���� Linux ��ʽ��·����
��������ֵ          �ѻؼ�����ĸ�Ŀ¼��ָ�롣 
*--------------------------------------------------------------------------*/
char *root_dir(char *dir) {

  char *p, *q;

  /* �����Ե�ǰ����Ŀ¼Ϊ��Ŀ¼ */
  dir =  (dir[0] == '\\' && dir[1] == '\\') ? dir + 2 : dir;
  q = dir;
  while((p = strchr(q, '\\')) != NULL) {
    *p = '/';
    q = p;
  }
  return dir;
} /* root_dir */

/*--------------------------------------------------------------------------*
��������            efprintf     - ��ӡ������Ϣ 
����ԭ��            int efprintf(FILE *stream, char *fmt, ...);
��������ͷ�ļ�      auxi.h
                    ��ӡ������Ϣ.��streamΪNULLʱ���������׼����
��������ֵ          ����ʵ�ʴ�ӡ���ֽ���,����ʱ���� -1.
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
��������            sysreboot            - ��������
����ԭ��            void sysreboot(void)
��������ͷ�ļ�      auxi.h
                    ������������ϵͳ.
��������ֵ          ��.
*--------------------------------------------------------------------------*/
void sysreboot(void) {

  sync();
  reboot(RB_AUTOBOOT);
} /* sysreboot */
