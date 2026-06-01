/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
// struct termios 、 tcgetattr() 、 tcsetattr() 、 ECHO 和 TCSAFLUSH 均源自
// <termios.h> 。

/*** define **ellisonleao/gruvbox.nvim*/

#define CTRL_KEY(k) ((k) & 0x1f)
/*** data  ***/
// 这是用户终端的原始属性，若没有，该程序可能会导致用户的终端属性被修改
struct editorConfig{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
struct editorConfig E;
/*** terminal ***/
void die(const char *s) {
    //退出时清屏
  write(STDOUT_FILENO,"\x1b[2J",4);
  write(STDOUT_FILENO,"\x1b[H",3);

  perror(s);
  exit(1);
}
void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}
void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");
  // atexit来自stdlib，用来注册函数，这样就是在程序退出的时候调用
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

  // c_lflag对应的是本地标志, 可以理解成杂项标志 raw.c_lflag &= ~(ECHO);
  // ICANON用于关闭规范模式，可以逐字节读取
  // ECHO用于在用户输入之后打印出用户的输入
  // ISIG禁用C-c和C-z
  // IXON控制C-s 和C-q,用于关闭软件流控制
  // IEXTEN：C-v会使终端等待你输入另一个字符，使用这个标志位来关闭
  // ICRNL用修复C-m，这个组合键被读取为10,但是预期应该是13,因为终端会
  // 将换行符（13）替换成新行符（10）
  raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cflag |= (CS8);

  // 设置read超时
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}
//获取光标位置
int getCursorPosition(int *rows,int *cols){
    char buf[32];
    unsigned int i = 0;
    if(write(STDOUT_FILENO ,"\x1b[6n",4)!=4)return -1;

    while(i < sizeof(buf) -1 )
    {
        if(read(STDIN_FILENO,&buf[i],1) != 1)break;
        if(buf[i] == 'R')break;
        i++;
    }
    buf[i] = '\0';

    if(buf[0] != '\x1b' || buf[1]!= '[')
        return -1;
    if(sscanf(&buf[2],getWindowSize"%d;%d",rows,cols)!=2)
        return -1;
    return 0;
}
int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;
    //调用ioctl并传入tiocgwinsz，并存到ws中
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws) == -1 || ws.ws_col == 0)
    {
        //用获取光标位置的方式来获取到我们需要的行列数
        if(write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12)!= 12)return -1;
        return getCursorPosition(rows,cols);
    }else{
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
/*** append buffer ***/

//支持追加的字符串
struct abuf{
    char *b;
    int len;
};
#define ABUF_INIT {NULL,0}

void abAppend(struct abuf *ab, const char *s,int len)
{
    char *new = realloc(ab->b,ab->len +len);
    if(new ==NULL)
        return;

    memcpy(&new[ab->len],s,len);
    ab->b = new;
    ab->len += len;
}
void abfree(struct abuf *ab)
{
    free(ab->b);
}
/*** output ***/

//绘制波浪线
void editorDrawRows(){
    int y ;
    for(y = 0;y<E.screenrows;y++)
    {
        wirte(STDOUT_FILENO,"~",1);
        if(y < E.screenrows - 1)
        write(STDOUT_FILENO,"\r\n",2);
    }
}
void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H",3);

    editorDrawRows();
    write(STDOUT_FILENO,"\x1b[H",3);
}

/*** input ***/

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO,"\x1b[2J",4);

    write(STDOUT_FILENO,"\x1b[H",3);


    exit(0);
    break;
  }
}
/*** init ***/
int initEditor(){
    if(getWindowSize(&E.screenrows,&E.screencols) ==-1)
        die("getWindowSize");
}
int main() {
  enableRawMode();
  initEditor();
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}

