/*** includes ***/
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
//struct termios 、 tcgetattr() 、 tcsetattr() 、 ECHO 和 TCSAFLUSH 均源自 <termios.h> 。

/*** data  ***/
//这是用户终端的原始属性，若没有，该程序可能会导致用户的终端属性被修改
struct termios orig_termios;

/*** terminal ***/
void die(const char* s)
{
    perror(s);
    exit(1);
}
void disableRawMode(){
  if(  tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios)== -1)
        die("tcsetattr");

}
void enableRawMode(){
    if(tcgetattr(STDIN_FILENO,&orig_termios) == -1)
        die("tcgetattr");
    //atexit来自stdlib，用来注册函数，这样就是在程序退出的时候调用
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    //c_lflag对应的是本地标志, 可以理解成杂项标志 raw.c_lflag &= ~(ECHO);
    //ICANON用于关闭规范模式，可以逐字节读取
    //ECHO用于在用户输入之后打印出用户的输入 
    //ISIG禁用C-c和C-z
    //IXON控制C-s 和C-q,用于关闭软件流控制
    //IEXTEN：C-v会使终端等待你输入另一个字符，使用这个标志位来关闭
    //ICRNL用修复C-m，这个组合键被读取为10,但是预期应该是13,因为终端会
    //将换行符（13）替换成新行符（10）
    raw.c_lflag &= ~(IXON| ICRNL|BRKINT| INPCK|ISTRIP); 
    raw.c_lflag &= ~(OPOST); 
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_lflag |= (CS8);

    //设置read超时
    raw.c_cc[VMIN]= 0;
    raw.c_cc[VTIME]=1;
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw)==-1)
        die("tcsetattr");
}

/*** init ***/
int main(){
    enableRawMode();
    char c;
    while(1){
        char c = '\0';
        if(read(STDIN_FILENO,&c,1)==-1 && errno != EAGAIN)die("read");
        if(iscntrl(c))
        {
            //如果是控制字符，就只打印它的编号
            printf("%d\r\n",c);
        }else {
            printf("%d ('%c')\r\n", c,c);
        }
        if(c =='q')break;
    }
    return 0;
}
