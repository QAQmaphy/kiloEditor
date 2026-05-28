#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
//struct termios 、 tcgetattr() 、 tcsetattr() 、 ECHO 和 TCSAFLUSH 均源自 <termios.h> 。

//这是用户终端的原始属性，若没有，该程序可能会导致用户的终端属性被修改
struct termios orig_termios;

void disableRawMode(){
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios);

}
void enableRawMode(){
    tcgetattr(STDIN_FILENO,&orig_termios);
    //atexit来自stdlib，用来注册函数，这样就是在程序退出的时候调用
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    //c_lflag对应的是本地标志, 可以理解成杂项标志 raw.c_lflag &= ~(ECHO);
    //ICANON用于关闭规范模式，可以逐字节读取
    //ECHO用于在用户输入之后打印出用户的输入 
    //ISIG禁用C-c和C-z
    //IXON控制C-s 和C-q,用于关闭软件流控制
    //IEXTEN：C-v会使终端等待你输入另一个字符，使用这个标志位来关闭
    raw.c_lflag &= ~(IXON);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
}

int main(){
    enableRawMode();
    char c;
    while(read(STDIN_FILENO,&c,1) == 1 &&  c!= 'q'){
        //iscntrl from ctype，用于判断一个字符是不是控制字符
        if(iscntrl(c))
        {
            //如果是控制字符，就只打印它的编号
            printf("%d\n",c);
        }else {
            printf("%d ('%c')\n", c,c);
        }
    }
    return 0;
}
