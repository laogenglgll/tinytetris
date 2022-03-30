#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


int 
     x = 431424,   // 方块x轴坐标
     y = 598356,   // 方块y轴坐标
     r = 427089,   // 某一种方块的具体那种形式 ( 同一个类别的方块可以经过旋转)
     px = 247872,  // 记录上一次的x 信息
     py = 799248,  // 记录上一次的y 信息  
	 pr,           // 记录上一次的r 信息
     c = 348480,   // 中间变量
     p = 615696,   // 记录方块的类别， 也记录了每种方块的颜色信息
     tick,         // 控制帧率  控制下落的速度
     board[20][10],// 整个地图(画面) 有无方块、方块的颜色信息
	 //{h-1,w-1}{x0,y0}{x1,y1}{x2,y2}{x3,y3} (two bits each)
     block[7][4] = {  {x, y, x, y},    // 打表  总共有7种类别的方块  每种类别可以经过旋转
                      {r, p, r, p},    // 每种方块 由四个子块组成
                      {c, c, c, c},    // 宽度-1 宽度-1, 高度-1 ， 四个子方块的相对位置   
				      {599636, 431376, 598336, 432192}, // 每个数字 仅仅低2位有效
                      {411985, 610832, 415808, 595540},
                      {px, py, px, py},
                      {614928, 399424, 615744, 428369}},
    score = 0;    //得分
    

/* 黑色 0   深蓝 1   深绿 2    深蓝绿 3   深红 4    紫 5     暗绿 6     
   白 7     灰 8     亮蓝 9    亮绿 10    亮蓝绿 11  红12    粉   13
   黄 14    亮白 15 
*/
/****************************
@func  设置控制台输出的颜色
@param bk 背景色
@param fg 前景色 
如果 bk == 0 and fg == 0 则为颜色恢复 
****************************/
int setcolor(int bk, int fg) 
{
	if(bk+fg == 0){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED |
									FOREGROUND_GREEN |
									FOREGROUND_BLUE);
	}else{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),(bk&0x0f)<<4+(fg&0x0f));
	}
	
	return 1;
}

void close_cur(){
	HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);//获得输出句柄 
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle,&CursorInfo);//获取控制台光标信息
	CursorInfo.bVisible=false;//隐藏控制台光标
	SetConsoleCursorInfo(handle,&CursorInfo);//设置控制台光标状态 
}
      
/****************
@func: 设置光标的位置
@param y 
@param x 
****************/
void move(int y,int x) 
{
	COORD pos;
	pos.X=x;
	pos.Y=y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
}

/*******************
@func:  绘制边界 
*******************/
void border(){
	int i=0;
	move(0,0);
	for(i = 1 ;i<22;i++) printf("-");
	move(21,0); 
	for(i = 1 ;i<22;i++) printf("-");
	for(i =0;i<22;i++){
		move(i,0);
		printf("|");
		move(i,21);
		printf("|");
	}
} 	
							
/*****************************
@param  x    某种类型方块的具体形式
@param  y    要取的两位为y+1 和 y+2
return  int  某两位的值
说明：  p决定是那种方块
******************************/
int NUM(int x, int y) { return 3 & block[p][x] >> y; }

/*******************************
@func:  产生一个新的方块
*******************************/
void new_piece() {
  y = py = 0;          // 新的块的y是0
  p = rand() % 7;      // 块的类别是7种的随机一种                                                       
  r = pr = rand() % 4; // 旋转类型也是四种的随机一种
  x = px = rand() % (10 - NUM(r, 16)); // 17,18位   x坐标是屏幕宽度-块的宽度之间的随机数
}

/*****************************
@func遍历 board 数组，在相应的位置画出对应颜色的块
*****************************/
void frame() {
  for (int i = 0; i < 20; i++) {
    move(1 + i, 1); // 到新的一行
    for (int j = 0; j < 10; j++) {
      if(board[i][j]){ 
	    // 如果board[i][j]非0 则进行后面的语句
      	setcolor(board[i][j],0);
		printf("  ");  
	  }else{
	  	setcolor(0,0);
	  	printf("  ");
	  }
    }
  }
  move(22, 1); 
  setcolor(0,0);
  printf("Score: %d", score);    
}

/******************************
@func： 给board数组(地图) 赋值， 在地图上表示出某个方块的位置信息
@param  x  要绘制的x坐标
@param  y  要绘制的y坐标
@param  r  旋转的类型
@param  v  具体的值， 非0 表示方块的颜色信息
                        0  表示擦除这个方块
******************************/
void set_piece(int x, int y, int r, int v) {
  for (int i = 0; i < 8; i += 2) { // 每个数据由2位表示，所以步进为2
    board[NUM(r, i * 2) + y][NUM(r, (i * 2) + 2) + x] = v;
  }
}

/********************************
@func: 擦除旧的方块在新的位置绘制
********************************/
int update_piece() {
  set_piece(px, py, pr, 0);                // 擦除原位置，在判断是否能够放下的时候要把自己影响消除，置0
  set_piece(px = x, py = y, pr = r, p + 1);// 块的类别加1为颜色信息， x,y为当前的新位置
  return 0;
}


/********************************
@func: 判断一行是否已经满了， 如果满了清除满的行，并得分
********************************/
void remove_line() {
  for (int row = y; row <= y + NUM(r, 18); row++) {  // 遍历的范围会当前方块的顶部到底部
    c = 1;
    for (int i = 0; i < 10; i++) { // 遍历一行
      c *= board[row][i];
    }
    if (!c) { // 为 0 表示一行中出现了0，没有满
      continue;
    }
    for (int i = row - 1; i > 0; i--) {
      memcpy(&board[i + 1][0], &board[i][0], 40); //如果满了，就将 上一行，依次向下一行移
    }
    memset(&board[0][0], 0, 10); //将最顶行清空
    score++; // 得分 
  }
}

/*********************************
@func: 判断将方块放置在x,y位置是否会冲突
*********************************/
int check_hit(int x, int y, int r) {
  if (y + NUM(r, 18) > 19) { // 方块的底部已经超出了屏幕
    return 1;
  }
  set_piece(px, py, pr, 0);// 清空当前方块，清除自己的影响
  c = 0;
  for (int i = 0; i < 8; i += 2) {
    board[y + NUM(r, i * 2)][x + NUM(r, (i * 2) + 2)] && c++; 
	// 判断本方块要放置的四个位置 是否已经有了别的方块
  }
  set_piece(px, py, pr, p + 1);// 在原位置重新放置
  return c; // 如果c == 0 表示可以放置
}


/*********************************
func:  控制方块下落的速度
*********************************/
int do_tick() {
  if (++tick > 30) {
  	Sleep(10);
    tick = 0;
    if (check_hit(x, y + 1, r)) {   
	  // 不能放置到下一行
      if (!y) {  // y == 0 游戏结束
        return 0;
      }
      remove_line(); // 判断是否已经满了
      new_piece(); // 在开头创建新的块
    } else {
	  // 可以放置到下一行
      y++;  
      update_piece();
    }
  }
  return 1;
}

/********************************
@func: 游戏主循环，  wasd 按键处理   w: 旋转
********************************/
void runloop() {
  while (do_tick()){
  	if(kbhit()){
	  	if ((c = getch()) == 'a' && x > 0 && !check_hit(x - 1, y, r)) {
		  // 左
	      x--;
	    }
	    if (c == 'd' && x + NUM(r, 16) < 9 && !check_hit(x + 1, y, r)) {
	      // 右
	      x++;
	    }
	    if (c == 's') {
	      while (!check_hit(x, y + 1, r)) { // 向下没有冲突就一直向下
	        y++;
	        update_piece();
	      }
	      remove_line();
	      new_piece();
	    }
	    if (c == 'w') {
	      ++r %= 4; // 旋转
	      while (x + NUM(r, 16) > 9) {  // 横坐标超出了边界
	        x--;
	      }
	      if (check_hit(x, y, r)) { // 如果变型后发生了碰撞，则不能变形，x还是原来的x, r还是原来的r
	        x = px;
	        r = pr;
	      }
	    }
	    if (c == 'q') {
	      return;
	    }
	}
    update_piece();
    frame();
  }
}

/*******************
@func main function  初始化
*******************/
int main() {
  srand(time(0));   // 初始化随机数种子
  SetConsoleTitle("Tetris");
  border();
  new_piece();
  close_cur();                // 关闭光标 
  runloop();
  return 0; 
}

