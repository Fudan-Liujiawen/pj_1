#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include <ctype.h>

// 地块定义
#define EMPTY 0
#define WALL 1
#define TRAP 2
#define TREASURE 3

// 玩家符号
#define PLAYER 'Y'
// 地图信息
int row_num,column_num;
char map[21][21] = {0};
// 地图
int sum_treasures;  // 宝藏数量
int level=0; //当前关卡s
int mode;  // 游戏模式

// 玩家状态
int x=1, y=1;         // 玩家当前坐标
int energy=0;      // 体力值
int energy_add;      //每一步消耗的体力
int get_treasures=0;    // 找到的宝藏数量
int flag=0;    //上一步是否踩入陷阱标记
int step=0;    //记录有效移动步数
char *route;    //记录玩家移动路径
int size=500;    //路径信息默认最大长度
typedef struct{  //定义栈结构体类型(撤销恢复)
        char *aMove;  //行动路径数组
        int *aEnergy;  //体力消耗数组
        int top;
        int capacity;  // 新增容量字段
    } ur_stack;
int capacity=500;  //撤销栈和重做栈默认长度
ur_stack* init_ur_stack(int capacity){    //初始化栈结构体
    ur_stack *stack = (ur_stack*)malloc(sizeof(ur_stack));
    stack->aMove = (char*)malloc(sizeof(char)*capacity);
    stack->aEnergy = (int*)malloc(sizeof(int)*capacity);
    stack->top = -1;
    stack->capacity = capacity;  // 初始化容量
    return stack;
}
int push(ur_stack *stack,char move,int energy){    //入栈
    if (stack->top + 1 >= stack->capacity) {
        stack->capacity *= 2;
        stack->aMove = (char*)realloc(stack->aMove, sizeof(char) * stack->capacity);
        stack->aEnergy = (int*)realloc(stack->aEnergy, sizeof(int) * stack->capacity);
    }
    stack->top++;
    stack->aMove[stack->top] = move;
    stack->aEnergy[stack->top] = energy;
    return 0;
} 
int pop(ur_stack *stack,char *move,int *energy){    //出栈
    if(stack->top==-1){
        return -1;
    }
    *move = stack->aMove[stack->top];
    *energy = stack->aEnergy[stack->top];
    stack->top--;
    return 0;
}
void clear_stack(ur_stack *stack){
    stack->top = -1;
}
void free_stack(ur_stack *stack) {
    free(stack->aMove);
    free(stack->aEnergy);
    free(stack);
}



// 结束状态
int end_type;
  
// 开始界面
/*
小黄的奇妙探险！

> 开始第一关
  开始第二关
  退出

控制方法：按W向上移动，按S向下移动，按<Enter>选择。
*/
void render_start(int select) {
    system("cls");
    printf("小黄的奇妙探险！\n\n");                              
    
    if(select == 0){
        printf("> 开始第一关\n");
    }
    else{
        printf("  开始第一关\n");
    }
    if(select == 1){
        printf("> 开始第二关\n");
    }
    else{
        printf("  开始第二关\n");
    }
    if(select == 2){
        printf("> 退出\n");
    }
    else{
        printf("  退出\n");
    }
    printf("\n");
    printf("控制方法：按W向上移动，按S向下移动，按<Enter>选择。\n");
}
// 关卡选择
void choose_level() {
    int select=0;
    while(1){
        render_start(select);
        char key = getch();
        if(key=='w'){
            if(select>0){
                select--;
            }
        }
        if(key=='s'){
            if(select<2){
                select++;
            }
        }
        if(key == '\r' || key == '\n'){
            break;
        }
    }

    if(select==0){
        level=1;
    }
    if(select==1){
        level=2;
    }
    if(select==2){
        exit(0);
    }
    
    }

// 模式选择界面
void choose_mode_render(int select){
    system("cls");
    printf("请选择控制模式：\n");
    if(select==0){
        printf("> 0: 实时模式\n");
    }
    else{
        printf("  0: 实时模式\n");
    }
    if(select==1){
        printf("> 1: 编程模式\n");
    }
    else{
        printf("  1: 编程模式\n");
    }
    printf("\n");
    printf("控制方法：按W向上移动，按S向下移动，按<Enter>选择。");
}


// 模式选择
void choose_mode() { 
    int select = 0;
    
    while(1){
        choose_mode_render(select);
        char input=getch();
        if(input=='w'){
            if(select>0){
                select--;
            }
        }
        if(input=='s'){
            if(select<1){
                select++;
            }
        }
        if(input == '\r' || input == '\n'){
            break;
        }
    }
    mode = select;  
}

// 初始化地图（硬编码或者从文件中加载）
void init_map(int level) {
    char map_1[15][15] = {
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
        {'W', 'Y', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', ' ', ' ', ' ', 'W'},
        {'W', ' ', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W'},
        {'W', ' ', ' ', ' ', 'W', ' ', ' ', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W'},
        {'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W'},
        {'W', ' ', ' ', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W'},
        {'W', ' ', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', ' ', ' ', ' ', 'T', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W'},
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W'},
        {'W', ' ', ' ', ' ', 'W', ' ', ' ', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W'},
        {'W', ' ', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W', ' ', 'W'},
        {'W', ' ', ' ', ' ', ' ', ' ', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W'},
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'}
    };
    char map_2[21][21] = {
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
        {'W', 'Y', ' ', ' ', ' ', ' ', 'W', 'D', 'D', ' ', 'W', ' ', ' ', ' ', ' ', 'D', ' ', 'D', ' ', ' ', 'W'},
        {'W', 'W', 'W', 'W', 'W', ' ', 'W', ' ', 'W', 'D', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W'},
        {'W', ' ', 'D', ' ', 'W', ' ', ' ', 'D', 'W', ' ', ' ', 'D', ' ', ' ', 'W', ' ', ' ', 'D', 'W', ' ', 'W'},
        {'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', ' ', ' ', 'D', ' ', 'W', ' ', ' ', ' ', ' ', ' ', 'W', 'D', 'W', ' ', ' ', ' ', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', 'W', 'W', 'D', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W', 'D', 'W', ' ', ' ', ' ', 'W', ' ', 'W', ' ', ' ', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W', 'D', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'D', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', ' ', ' ', ' ', ' ', ' ', 'D', 'D', ' ', ' ', ' ', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'D', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', ' ', ' ', 'W', ' ', ' ', 'D', ' ', ' ', ' ', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', ' ', ' ', 'W', ' ', ' ', ' ', 'W', ' ', ' ', ' ', 'W', ' ', 'W', 'D', ' ', ' ', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W', 'D', 'W', 'W', 'W', ' ', 'W', 'W', 'W', 'W', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', 'W', 'D', 'W', ' ', ' ', ' ', 'W', ' ', 'W', 'D', 'W', ' ', ' ', ' ', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W', ' ', 'W', ' ', 'W', ' ', 'W', ' ', 'W', 'W', 'W'},
        {'W', ' ', 'W', ' ', 'W', 'T', 'W', ' ', ' ', 'D', 'W', ' ', 'W', ' ', ' ', ' ', 'W', ' ', 'W', 'D', 'W'},
        {'W', ' ', 'W', ' ', 'W', ' ', 'W', 'D', 'W', ' ', 'W', 'D', 'W', 'W', 'W', 'W', 'W', ' ', 'W', ' ', 'W'},
        {'W', ' ', 'W', ' ', ' ', ' ', 'D', 'D', 'W', ' ', 'W', 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W'},
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'}
    };
    
    if (level==1){
        for(int i=0;i<15;i++){
            for(int j=0;j<15;j++){
                map[i][j]=map_1[i][j];
            }
        }
        row_num=15;
        column_num=15;
        sum_treasures=1;
    }
    if (level==2){
        memcpy(map,map_2,sizeof(map_2));
        row_num=21;
        column_num=21;
        sum_treasures=2;
    }


    

    // 地图信息初始化  
}

// 初始化玩家状态
void init_player(){
    x=1;
    y=1;
    energy=0;
    get_treasures=0;
    step=0;
    flag=0;
   // 初始化玩家位置和体力，找到的宝藏数量,路径信息
}

// 渲染地图
void render_map() {
    system("cls");
    for(int i=0;i<row_num;i++){
        for(int j=0;j<column_num;j++){
            printf("%c ",map[i][j]);
        }
        printf("\n");
    }  
}

// 展示信息
void render_info(){
    printf("体力消耗：%d\n",energy);
    printf("控制方法：按W向上移动，按S向下移动，按A向左移动，按D向右移动，按I原地不动，按Q结束冒险。"); 
}

// 结束界面
void render_end() { 
    system("cls");
    switch(end_type){
        case 0:
            printf("主动结束！");break;
        case 1:
            printf("恭喜你，小黄找到了所有宝藏!");break;
        case 2:
            printf("非法输入终止！");break;
    }
    
    printf("\n\n行动路径：%s\n",route);
    printf("消耗的体力：%d\n",energy);
    printf("找到的宝箱的数量：%d\n\n",get_treasures);
    printf("<按任意键继续>\n");

}

// 检查移动是否合法
bool is_valid_move(char input) {
    switch(input){
        case 'w':
            return map[x-1][y]==' ' || map[x-1][y]=='D';
        case 's':
            return map[x+1][y]==' ' || map[x+1][y]=='D';
        case 'a':
            return map[x][y-1]==' ' || map[x][y-1]=='D';
        case 'd':
            return map[x][y+1]==' ' || map[x][y+1]=='D';
        default: return false;
    }
}

// 检查附近是否有宝藏并收取
bool check_for_treasure() { 
    if(map[x-1][y]=='T'){
        map[x-1][y]=' ';
        return true;
    }
    if(map[x+1][y]=='T'){
        map[x+1][y]=' ';
        return true;
    }
    if(map[x][y-1]=='T'){
        map[x][y-1]=' ';
        return true;
    }
    if(map[x][y+1]=='T'){
        map[x][y+1]=' ';
        return true;
    }
    return false;
}

// 玩家移动
void move_player(char input) {
    map[x][y]=' ';
    if(flag==1){
        map[x][y]='D';
        flag=0;
    }
    switch(input){
        case 'w': x--;break;
        case 's': x++;break;
        case 'a': y--;break;
        case 'd': y++;break;
    }
    if(map[x][y]=='D'){
        flag=1;
    }
    map[x][y]='Y';
}

// 记录玩家移动
void record_player_path(char input) {
    if(step+1>=size){
        size*=2;
        route = (char *)realloc(route,size*sizeof(char));
    }
    switch(input){
        case 'w': route[step]='U';step++;route[step]='\0';break;
        case 's': route[step]='D';step++;route[step]='\0';break;
        case 'a': route[step]='L';step++;route[step]='\0';break;
        case 'd': route[step]='R';step++;route[step]='\0';break;
    }
}


// 记录体力消耗
void energy_sum(char input) {
    energy_add=0;
    if (input=='w'||input=='a'||input=='s'||input=='d'||input=='i'){
        energy_add=1;
        if(flag==1){
            energy_add++;
        }
    }
    energy+=energy_add;
}

// 游戏循环
void game_loop() {
    int is_invalid=0;  // 输入是否合法标志
    int is_invalid_undo=0;  //撤销是否合法标志
    int is_invalid_redo=0;  //重做是否合法标志
    ur_stack *undostack = init_ur_stack(capacity);
    ur_stack *redostack = init_ur_stack(capacity);
    if(mode==0){
        while(1){
            render_map();//渲染地图
            render_info();
            if(is_invalid){
                printf("输入错误，请重新输入");
                is_invalid=0;
            }
            if(is_invalid_undo){
                printf("没有更多可撤销的步骤！");
                is_invalid_undo=0;
            }
            if(is_invalid_redo){
                printf("已恢复至最新步骤！");
                is_invalid_redo=0;
            }
            char input=tolower(getch());//获取输入
            if(!(input=='w'||input=='a'||input=='s'||input=='d'||input=='i'||input=='q'||input=='z'||input=='y')){  //判断输入是否合法,非法则直接跳过本次循环
                is_invalid=1;
                continue;
            }
            energy_sum(input);//记录体力消耗
            if(is_valid_move(input)){ //判断移动是否合法
                clear_stack(redostack);  //清空重做栈
                move_player(input);  //玩家移动
                
                push(undostack,input,energy_add);  //入撤销栈
                record_player_path(input);  //记录玩家移动
                if(check_for_treasure()){  //判断附近是否有宝藏
                    get_treasures++;
                }
                
            }
            
            
            else if(input=='q'){     //用户按q键结束
                end_type=0;
                printf("\n(检测到主动结束)按任意键确定结束");
                getch();
                break;
            }

            else if(input=='z'){    //用户按z键执行撤销操作
                char _move;
                int _energy, _isind;
                if(pop(undostack,&_move,&_energy)==-1){  //出撤销栈
                    is_invalid_undo=1;
                    continue;
                }
                map[x][y]=' ';
                if(flag==1){
                    map[x][y]='D';
                    flag=0;
                }
                switch(_move){
                    case 'w': x++;break;
                    case 's': x--;break;
                    case 'a': y++;break;
                    case 'd': y--;break;
                }
                push(redostack,_move,_energy);  //入重做栈
                if(map[x][y]=='D'){  //检查撤销步骤执行后是否落入陷阱
                    flag=1;
                }
                //printf("%d ",energy);错误检查
                //printf("%d ",_energy);错误检查
                energy-=_energy;
                //printf("%d",energy);错误检查
                
                map[x][y]='Y';
                //char csdc=getchar();错误检查
                
                route[step]='\0';//删除最后一步路径信息
                step--;
            }

            else if(input=='y'){
                char _move;
                int _energy;
                if(pop(redostack,&_move,&_energy)==-1){  //出重做栈
                    is_invalid_redo=1;
                    continue;
                }
                map[x][y]=' ';
                if(flag==1){
                    map[x][y]='D';
                    flag=0;
                }
                switch(_move){
                    case 'w': x--;break;
                    case 's': x++;break;
                    case 'a': y--;break;
                    case 'd': y++;break;
                }
                if(map[x][y]=='D'){  //检查重做步骤执行后是否落入陷阱
                    flag=1;
                }
                map[x][y]='Y';
                energy+=_energy;
                push(undostack,_move,_energy);  //再入撤销栈

                record_player_path(_move);  //重新写入路径信息
            }

            
            if(get_treasures==sum_treasures){
                end_type=1;
                render_map();//渲染地图
                render_info();
                printf("\n您已找到全部宝藏，按任意键确定结束");
                getch();
                break;
            }
        }
        free_stack(undostack);
        free_stack(redostack);

    }

    if(mode==1){
        while(1){
            render_map();//渲染地图
            render_info();
            printf("\n请输入你的操作序列：");
            char input=tolower(getchar());//获取输入
            if (input == '\n') {
                continue;
        }
            if(!(input=='w'||input=='a'||input=='s'||input=='d'||input=='i'||input=='q')){  //判断输入是否合法
                end_type=2;
                render_map();//渲染地图
                render_info();
                printf("\n(检测到非法输入)按任意键确定结束");
                char tempt;
                while((tempt=getchar())!='\n' && tempt!=EOF);//使用 getchar 循环清空缓冲区
                getch();//保持当前界面，在用户按任意键后继续
                break;
            }
            energy_sum(input);//记录体力消耗
            if(is_valid_move(input)){ //判断移动是否合法
                move_player(input);  //玩家移动
                record_player_path(input);  //记录玩家移动
                if(check_for_treasure()){  //判断附近是否有宝藏
                    get_treasures++;
                }
            }
            
            
            else if(input=='q'){
                end_type=0;
                render_map();//渲染地图
                render_info();
                printf("\n(检测到主动结束)按任意键确定结束");
                getch();
                break;
            }
            if(get_treasures==sum_treasures){
                end_type=1;
                render_map();//渲染地图
                render_info();
                printf("\n您已找到全部宝藏，按任意键确定结束");
                getch();
                break;
            }
        }
    }

}








// 游戏入口
int main() {
    SetConsoleOutputCP(65001);
    while(1){
        init_player();
        route = (char *)malloc(size*sizeof(char));   //为路径信息分配内存
        route[0]='\0';    //路径信息初始化
              
        choose_level();
        init_map(level);
        
        choose_mode();
        
        game_loop();
        
        render_end();
        
        free(route);
        
        getch();
        
    }
    return 0;
}
