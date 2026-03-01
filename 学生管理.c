#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 20
#define MAX_NAME_LEN 50
#define MAX_PWD_LEN 30
#define DATA_FILE "students.dat"   // 二进制数据文件
#define USER_FILE "users.dat"      // 用户账号文件
#define PAGE_SIZE 5              // 每页显示行数

#define ReGister   1
#define PwdReset    2//密码找回和密码重置时都在2里面，或者说，密码找回就是密码重置
#define Appeal      3

typedef struct Student {
    char id[MAX_ID_LEN];        // 学号
    char name[MAX_NAME_LEN];    // 姓名
    char gender[10];            // 性别
    int age;                    
    float score_c;              
    float score_math;           
    float score_eng;            
    float total;                
    float average;             
    
    struct Student *prev;       
    struct Student *next;       
} Student;

typedef struct User {
    char username[30];
    char password[MAX_PWD_LEN];
    char id[MAX_ID_LEN];
    int role; //1:学生, 2:教师, 3:管理员
} User;

typedef struct {
    int id;             //流水号
    int type;           //类型
    char sender[30];    //申请人ID
    char content[200];  //申诉内容
    int status;
} ToDoItem;

//全局变量
Student *head = NULL;
User currentUser;       

// 清除输入缓冲区
void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 1. 输入辅助

void readString(char *buffer, int maxLen) {
    char temp[1024];
    if (fgets(temp, sizeof(temp), stdin)) {
        int len = strlen(temp);
        if (len > 0 && temp[len - 1] == '\n') {
            temp[len - 1] = '\0';
        }else {
            clearBuffer();//读取的超过maxlen了，最后一个不是换行符，要清除输入缓冲区
        }

        if (strlen(temp) >= maxLen) {
            temp[maxLen - 1] = '\0';
        }
        strcpy(buffer, temp);
    }
}

int readInt(char *prompt, int min, int max) {
    int val;
    char buffer[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin)) {
            //sscanf返回成功匹配并赋值的个数
            //从buffer匹配出整数并赋值给value
            if (sscanf(buffer, "%d", &val) == 1) {
                if (val >= min && val <= max) {
                    return val;
                }else {
                    printf("输入错误: 输入从 %d 到 %d 之间的整数。\n", min, max);
                }
            }else {
                printf("输入错误:输入有效的数字。\n");
            }
        }
    }
}

float readFloat(const char *prompt, float min, float max) {
    float val;
    char buffer[100];

    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin)) {
            if (sscanf(buffer, "%f", &val) == 1) {
                if (val >= min && val <= max) {
                    return val;
                }else {
                    printf("输入错误: 数值必须在 %f 和 %f 之间\n", min, max);
                }
            }else {
                printf("输入错误: 请输入有效数字\n");
            }
        }
    }
}

void pauseSystem() {
    printf("\n按回车键继续...");
    getchar();
}

//2. 
//增删改查、创建dummyHead、创建新节点和数据的计算

void initList() {
    head = (Student*)malloc(sizeof(Student));
    if (head == NULL) {
        printf("内存分配失败。\n");
        exit(1);  //程序终止，1为退出码（错误），0为正常退出
    }
    head->prev = NULL;
    head->next = NULL;
    strcpy(head->id, "HEAD");//标记一下虚拟头节点
}

Student* createNode() {
    Student *node = (Student*)malloc(sizeof(Student));
    if (node == NULL) return NULL;
    memset(node, 0, sizeof(Student));
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void calculateStats(Student *node) {
    if (node) {
        node->total = node->score_c + node->score_eng + node->score_math;
        node->average = node->total / 3.0;
    }
}

void appendStudent(Student *newNode) {
    Student *cur = head;
    while (cur->next) {
        cur = cur->next;
    }
    
    cur->next = newNode;
    newNode->next = NULL;
    newNode->prev = cur;
}

Student* findStudentById(const char *id) {
    Student *p = head->next;
    while (p) {
        if (strcmp(p->id, id) == 0) {
            return p;
        } 
        p = p->next;
    }
    return NULL;
}

int deleteStudentNode(const char *id) {
    Student *tar = findStudentById(id);
    if (tar == NULL) {
        return 0;
    }
    if (tar->prev) {
        tar->prev->next = tar->next;
    }
    if (tar->next ) {
        tar->next->prev = tar->prev;
    }

    free(tar);
    return 1; 
}

void freeAllStudents() {
    Student *p = head->next;
    Student *temp;
    while (p) {
        temp = p;
        p = p->next;
        free(temp);
    }
    head->next = NULL;
}

//3. 文件操作
void saveToBinaryFile() {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (fp == NULL) {
        printf("保存失败:无法打开文件%s\n", DATA_FILE);
        return;
    }

    Student *p = head->next;
    int cnt = 0;
    while (p) {
        int dataSize = sizeof(Student) - 2 * sizeof(struct Student*);
        fwrite(p, dataSize, 1, fp);
        p = p->next;
        cnt++;
    }
    fclose(fp);
    printf("成功保存 %d 条数据到文件\n", cnt);
}

//数据读取
void loadFromBinaryFile() {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (fp == NULL) {
        return;
    }

    freeAllStudents();

    Student temp;
    int dataSize = sizeof(Student) - 2 * sizeof(struct Student*);

    while (fread(&temp, dataSize, 1, fp)) {
        Student *newNode = createNode();
        //内存拷贝
        memcpy(newNode, &temp, dataSize);
        newNode->next = NULL;
        newNode->prev = NULL;
        //重新计算保证数据正确
        calculateStats(newNode);
        appendStudent(newNode);
    }
    fclose(fp);
    printf("系统启动:已加载历史数据\n");
}

//4. 排序
void swapNodeData(Student *a, Student *b) {
    Student temp;
    //temp = a
    strcpy(temp.id, a->id);
    strcpy(temp.name, a->name);
    strcpy(temp.gender, a->gender);
    temp.age = a->age;
    temp.score_c = a->score_c;
    temp.score_math = a->score_math;
    temp.score_eng = a->score_eng;
    temp.total = a->total;
    temp.average = a->average;

    //a = b
    strcpy(a->id, b->id);
    strcpy(a->name, b->name);
    strcpy(a->gender, b->gender);
    a->age = b->age;
    a->score_c = b->score_c;
    a->score_math = b->score_math;
    a->score_eng = b->score_eng;
    a->total = b->total;
    a->average = b->average;

    //b = temp
    strcpy(b->id, temp.id);
    strcpy(b->name, temp.name);
    strcpy(b->gender, temp.gender);
    b->age = temp.age;
    b->score_c = temp.score_c;
    b->score_math = temp.score_math;
    b->score_eng = temp.score_eng;
    b->total = temp.total;
    b->average = temp.average;
}

//冒泡排序
//sortType 1 = 总分 2 = c语言 3 = 数学 4 = 英语
//order 1 = 升序 2 = 降序
void sortStudents(int sortType, int order) {
    if (head->next == NULL) return;

    int swapped;
    Student *ptr1;
    Student *lptr = NULL;

    do {
        swapped = 0;
        ptr1 = head->next;

        while (ptr1->next != lptr) {
            int condition = 0;
            float val1, val2;

            switch (sortType) {
                case 1: val1 = ptr1->total; val2 = ptr1->next->total; break;
                case 2: val1 = ptr1->score_c; val2 = ptr1->next->score_c; break;
                case 3: val1 = ptr1->score_math; val2 = ptr1->next->score_math; break;
                case 4: val1 = ptr1->score_eng; val2 = ptr1->next->score_eng; break;
            }

            if (order == 1) condition = (val1 > val2); //升序(不符合升序的情况condition == 1)
            else condition = (val1 < val2); //降序

            if (condition) {
                swapNodeData(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);

    printf("排序完成\n");
}

//排序菜单
void menuSort() {
    int type, order;
    printf("\n=== 排序选项 ===\n");
    printf("1.按总分\n2. 按C语言\n3. 按数学\n4. 按英语\n");
    type = readInt("请选择排序依据(1-4):", 1, 4);

    printf("1. 升序\n2. 降序\n");
    order = readInt("请选择排序方式(1-2):", 1, 2);

    sortStudents(type, order);
    viewWithPagination();
}

// 5. 分页显示

//打印表头
void printTableHeader() {
    printf("\n%-10s %-10s %-6s %-4s %-6s %-6s %-6s %-6s %-6s\n", 
        "学号", "姓名", "性别", "年龄", "C语言", "数学", "英语", "总分", "平均分");
    printf("------------------------------------------------------------\n");
}

//每一行的内容
void printStudentRow(Student *p) {
    printf("%-10s %-10s %-6s %-4d %-6.1f %-6.1f %-6.1f %-6.1f %-6.1f\n",
           p->id, p->name, p->gender, p->age,
           p->score_c, p->score_math, p->score_eng, p->total, p->average);
}

//分页查看
void viewWithPagination() {
    if (head->next == NULL) {
        printf("还没有数据\n");
        pauseSystem();
        return;
    }

    Student *cur = head->next;
    int pageNumber = 1;

    while (1) {
        printf("\n\n\n\n\n");
        printf(">>> 第 %d 页 <<<\n", pageNumber);
        printTableHeader();

        Student *iter = cur;
        int cnt = 0;
        
        //打印当前页
        while (iter != NULL && cnt < PAGE_SIZE) {
            printStudentRow(iter);
            iter = iter->next;
            cnt++;
        }

        printf("--------------------------------------------------\n");
        printf("操作: [N]下一页 [P] 上一页 [Q]退出\n");
        printf("输入选择:");
        
        char cmd[10];
        readString(cmd, 10);

        if (cmd[0] == 'n' || cmd[0] == 'N') {
            Student *temp = cur;
            int step = 0;

            while (temp != NULL && step < PAGE_SIZE) {
                temp = temp->next;
                step++;
            }
            
            if (temp != NULL) {
                cur = temp;
                pageNumber++;
            }else {
                printf("已经是最后一页了\n");
                pauseSystem();
            }
        }else if (cmd[0] == 'p' || cmd[0] == 'P') {
            //cur指针必定是在每一页的开头，所以先检查前面有没有节点，有的话就肯定还有上一页
            if (cur->prev != head && cur->prev) {
                Student *temp = cur;
                int step = 0;
                while (temp->prev != head && step < PAGE_SIZE) {
                    temp = temp->prev;
                    step++;
                }
                cur = temp;
                pageNumber--;
            }else {
                printf("已经是第一页了");
                pauseSystem();
            }
        }else if (cmd[0] == 'q' || cmd[0] == 'Q') {
            break;
        }
    }
}

//6. 统计与成绩分析

//单个科目的条形统计图
//ranges装的每个级别的人数
void drawSubjectBar(const char *subName, int ranges[]) {
    printf("\n[%s成绩分布]\n", subName);
    char *lables[] = {"<60", "60-69", "70-79", "80-89", ">=90"};
    for (int i = 0; i < 5; i++) {
        printf("%s | ", lables[i]);
        //打印#号
        for (int j = 0; j < ranges[i]; j++) {
            printf("#");
        }
        printf("(%d)\n", ranges[i]);
    }
}

//综合统计
void performStatistics() {
    if (head->next == NULL) {
        printf("暂无数据\n");
        return;
    }

    int cnt = 0;
    float sumC = 0, sumM = 0, sumE = 0;
    float maxC = -1, maxM = -1, maxE = -1;
    float minC = 101, minM = 101, minE = 101;

    int distC[5] = {0}, distM[5] = {0}, distE[5] = {0};

    Student *p = head->next;
    while (p) {
        cnt++;
        sumC += p->score_c;
        sumM += p->score_math;
        sumE += p->score_eng;

        // 更新最值
        if (p->score_c > maxC) maxC = p->score_c;
        if (p->score_c < minC) minC = p->score_c;
        if (p->score_math > maxM) maxM = p->score_math;
        if (p->score_math < minM) minM = p->score_math;
        if (p->score_eng > maxE) maxE = p->score_eng;
        if (p->score_eng < minE) minE = p->score_eng;

        // 统计C语言分布
        if (p->score_c < 60) distC[0]++;
        else if (p->score_c < 70) distC[1]++;
        else if (p->score_c < 80) distC[2]++;
        else if (p->score_c < 90) distC[3]++;
        else distC[4]++;

        // 统计数学分布
        if (p->score_math < 60) distM[0]++;
        else if (p->score_math < 70) distM[1]++;
        else if (p->score_math < 80) distM[2]++;
        else if (p->score_math < 90) distM[3]++;
        else distM[4]++;
        
        // 统计英语分布
        if (p->score_eng < 60) distE[0]++;
        else if (p->score_eng < 70) distE[1]++;
        else if (p->score_eng < 80) distE[2]++;
        else if (p->score_eng < 90) distE[3]++;
        else distE[4]++;

        p = p->next;
    }
    printf("\n======= 班级成绩统计报告 =======\n");
    printf("总人数: %d\n", cnt);
    printf("C语言  - 平均: %5.2f | 最高: %5.1f | 最低: %5.1f | 及格率: %5.1f%%\n", 
           sumC/cnt, maxC, minC, (float)(cnt-distC[0])/cnt*100);
    printf("数学   - 平均: %5.2f | 最高: %5.1f | 最低: %5.1f | 及格率: %5.1f%%\n", 
           sumM/cnt, maxM, minM, (float)(cnt-distM[0])/cnt*100);
    printf("英语   - 平均: %5.2f | 最高: %5.1f | 最低: %5.1f | 及格率: %5.1f%%\n", 
           sumE/cnt, maxE, minE, (float)(cnt-distE[0])/cnt*100);

    printf("\n======= 图表分析 =======\n");
    drawSubjectBar("C语言", distC);
    drawSubjectBar("数学 ", distM);
    drawSubjectBar("英语 ", distE);
    
    pauseSystem();
}

//7. 信息管理与修改

//录入心学生
void addNewStudent() {
    printf("\n=== 录入新学生 ===\n");
    Student *node = createNode();

    while (1) {
        printf("输入学号(id):");
        readString(node->id, MAX_ID_LEN);
        if (strlen(node->id) == 0) {
            printf("学号不能为空！\n"); 
            continue;
        }
        else if (findStudentById(node->id)) {
            printf("学号已存在！\n"); 
            continue;
        }
        break;
    }

    printf("输入姓名:");
    readString(node->name, MAX_NAME_LEN);

    while(1) {
        printf("输入性别 (M/F): "); 
        readString(node->gender, 10);
        if (strcmp(node->gender, "M")==0 || strcmp(node->gender, "F")==0 || 
        strcmp(node->gender, "m")==0 || strcmp(node->gender, "f")==0) 
            break;
        printf("请输入 M 或 F。\n");
    }

    node->age = readInt("输入年龄 (15-40): ", 15, 40);
    node->score_c = readFloat("C语言成绩 (0-100): ", 0, 100);
    node->score_math = readFloat("数学成绩 (0-100): ", 0, 100);
    node->score_eng = readFloat("英语成绩 (0-100): ", 0, 100);

    calculateStats(node);
    appendStudent(node);

    printf("录入成功！\n");
    saveToBinaryFile();
    pauseSystem();
}

//修改学生信息 菜单
void modifyStudentMenu() {
    char id[MAX_ID_LEN];
    printf("请输入要修改的学号:");
    readString(id, MAX_ID_LEN);

    Student *p = findStudentById(id);
    if (!p) {
        printf("没找到该学生\n");
        pauseSystem();
        return;
    }

    int choice;
    while (1) {
        printf("\n=== 修改信息: %s (%s) ===\n", p->name, p->id);
        printf("1. 修改姓名\n");
        printf("2. 修改性别\n");
        printf("3. 修改年龄\n");
        printf("4. 修改各科成绩\n");
        printf("0. 完成修改\n");
        choice = readInt("请选择:", 0, 4);

        if (choice == 0) break;

        switch(choice) {
            case 1:
                printf("原姓名: %s, 新姓名: ", p->name);
                readString(p->name, MAX_NAME_LEN);
                break;
            case 2:
                printf("原性别: %s, 新性别: ", p->gender);
                readString(p->gender, 10);
                break;
            case 3: 
                printf("原年龄: %d\n", p->age);
                p->age = readInt("新年龄: ", 15, 40);
                break;
            case 4:
                p->score_c = readFloat("新C语言成绩: ", 0, 100);
                p->score_math = readFloat("新数学成绩: ", 0, 100);
                p->score_eng = readFloat("新英语成绩: ", 0, 100);
                calculateStats(p); // 重算总分
                break;
        }
        printf("更新成功。\n");
    }
    saveToBinaryFile();
}

//搜索菜单
void searchMenu() {
    int choice;
    printf("\n=== 查找学生 ===\n");
    printf("1. 按学号查找\n2. 按姓名查找\n");
    choice = readInt("选择:", 1, 2);

    if (choice == 1) {
        char id[MAX_ID_LEN];
        printf("输入学号: "); 
        readString(id, MAX_ID_LEN);
        Student *p = findStudentById(id);
        if (p) {
            printTableHeader();
            printStudentRow(p);
        }else {
            printf("没找到\n");
        }
    }else {
        char name[MAX_NAME_LEN];
        printf("输入姓名\n");
        readString(name, MAX_NAME_LEN);

        if (strlen(name) == 0) {
            printf("输入不能为空！\n");
            pauseSystem();
            return;
        }

        Student *p = head->next;
        int found = 0;
        printTableHeader();
        while (p) {
            //输入姓名的一部分就可以罗列出所有可能
            if (strstr(p->name, name)) {
                printStudentRow(p);
                found = 1;
            }
            p = p->next;
        }
        if (!found) {
            printf("没找到名字包含 %s 的学生", name);
        }
    }
    pauseSystem();
}

// 8. 账号模块

//注册新用户
void registerUser(int forceRole) {
    FILE *fp = fopen(USER_FILE, "ab+");
    if (!fp) {
        printf("无法打开用户文件\n");
        return;
    }

    User newUser;
    User temp;

    while (1) {
        printf("输入用户名:");
        readString(newUser.username, 30);
        if (strlen(newUser.username) == 0) continue;

        int exist = 0;
        rewind(fp);
        while (fread(&temp, sizeof(User), 1, fp)) {
            if (strcmp(temp.username, newUser.username) == 0) {
                exist = 1;
                break;
            }
        }
        if (exist) {
            printf("用户名已存在, 请重试。\n");
        }else {
            break;
        }
    }

    printf("请输入密码: ");
    readString(newUser.password, MAX_PWD_LEN);

    printf("请输入id: ");
    readString(newUser.id, MAX_ID_LEN);

    if (forceRole == 0) {
        newUser.role = readInt("设置权限 (1:学生, 2:教师, 3:管理员): ", 1, 3);
    }else {
        newUser.role = forceRole;
    }

    fwrite(&newUser, sizeof(User), 1, fp);
    fclose(fp);
    printf("账号注册成功!\n");
    pauseSystem();
}

//登录
int performLogin() {
    char user[30], pwd[MAX_PWD_LEN];
    User u;
    FILE *fp = fopen(USER_FILE, "rb");

    if (!fp) {
        printf("检测到系统首次运行，正在创建默认管理员...\n");
        fp = fopen(USER_FILE, "wb");
        User admin = {"admin", "123456", "0000", 3};
        fwrite(&admin, sizeof(User), 1, fp);

        //fwrite 写入的数据不会立刻保存到硬盘里
        //必须调用fclose
        fclose(fp);

        fp = fopen(USER_FILE, "rb");
        printf("默认账号:admin, 密码: 123456\n");
    }

    printf("\n=== 系统登陆 ===\n");
    printf("账号: ");
    readString(user, 30);
    printf("密码: ");
    readString(pwd, MAX_PWD_LEN);

    int logSuccess = 0;
    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(user, u.username) == 0 && strcmp(pwd, u.password) == 0) {
            printf("登陆成功！用户名: %s,角色(role): %d\n", u.username, u.role);
            currentUser = u;
            logSuccess = 1;
            break;
        }
    }
    fclose(fp);
    return logSuccess;
}

//管理员：管理用户列表
void listAllUsers() {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) return;

    User u;
    printf("--- 用户列表 ---\n");
    printf("%-20s %-10s\n", "用户名", "权限");

    while (fread(&u, sizeof(User), 1, fp)) {
        char roleName[30];
        if (u.role == 1) 
            strcpy(roleName, "学生");
        else if(u.role == 2)
            strcpy(roleName, "教师");
        else 
            strcpy(roleName, "管理员");
        printf("%-20s %-10s\n", u.username, roleName);
    }
    fclose(fp);
    pauseSystem();
}

//9. 待办事项管理

int getNewId() {
    FILE *fp = fopen("todo.dat", "rb");
    if (!fp) return 101;//第一次从101开始

    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fclose(fp);
        return 101;
    }

    //fseek函数参数的要求，防止取负数时发生溢出翻转
    //sizeof算出来是size_t无符号整型不能取负
    fseek(fp, -((long)sizeof(ToDoItem)), SEEK_END);

    ToDoItem temp;
    fread(&temp, sizeof(ToDoItem), 1, fp);

    fclose(fp);
    return temp.id + 1;
}

//添加待办事项
void addToDoItem(int type, char *sender, char *content) {
   int id = getNewId();
   ToDoItem t;
   t.id = id;
   t.type = type;
   t.status = 0;//默认状态
   strcpy(t.sender, sender);
   strcpy(t.content, content);

   FILE *fp = fopen("todo.dat", "ab");
   if (fp) {
        fwrite(&t, sizeof(ToDoItem), 1, fp);
        fclose(fp);
        printf("申请已提交，流水号: %d\n", id);
   }else {
        printf("文件无法打开\n");
        return;
   }
}

//管理员查看待办事项
void viewToDoItems() {
    FILE *fp = fopen("todo.dat", "rb");
    if (!fp) {
        printf("当前无待办事项\n");
        return;
    }

    ToDoItem item;
    printf("\n==== 代办事项列表 ====\n");
    printf("%-5s | %-10s | %-15s | %-30s\n", "ID", "类型", "申请人", "详情/内容");
    printf("--------------------------\n");

    while (fread(&item, sizeof(ToDoItem), 1, fp)) {
        if (item.status != 0) continue;

        printf("%-5d \n", item.id);

        switch(item.type) {
            case ReGister :
                printf("%-10s | %-15s | 申请密码: %s\n", "账号注册", item.sender, item.content);
                break;
            case PwdReset :
                printf("%-10s | %-15s | 申请密码: %s\n", "账号注册", item.sender, item.content);
                break;
            case Appeal :
                printf("%-10s | %-15s | 理由: %s\n", "成绩申诉", item.sender, item.content);
                break;
            default :
                printf("%-10s | %-15s | 理由: %s\n", "成绩申诉", item.sender, item.content);
        }
    }
    printf("====================================================\n");
    fclose(fp);
}

//管理员处理待办事项
//newStatus 1表示通过 2表示不通过
//输入管理员的ID
void updateToDoItem(int id, int newStatus) {
    FILE *fp = fopen("todo.dat", "rb+");
    if (!fp) return;

    ToDoItem item;
    int found = 0;
    while (fread(&item, sizeof(ToDoItem), 1, fp)) {
        if (item.id == id && item.status == 0) {
            found = 1;
            item.status = newStatus;
            fseek(fp, -(long)sizeof(ToDoItem), SEEK_CUR);
            fwrite(&item, sizeof(ToDoItem), 1, fp);
            printf("申请 (ID: %d)状态已更新", item.id);
            break;
        }
    }

    if (!found) {
        printf("未找到ID为%d的申请. \n", id);
    }
    fclose(fp);

    //做事情
    if (found && newStatus == 1) {
        if (item.type == ReGister) {
            User u;
            strcpy(u.username, item.sender);
            strcpy(u.password, item.content);
            strcpy(u.id, "0000");
            u.role = 1;

            FILE *fp = fopen("users.dat", "ab");

            if (fp == NULL) return;
            
            if (fwrite(&u, sizeof(User), 1, fp)) {
                fclose(fp);
            }else {
                printf("写入失败\n");
                fclose(fp);
        }
    }

    else if (item.type == PwdReset) {
        User newU;
        FILE *fp = fopen(USER_FILE, "rb+");
        if (!fp) return;

        while (fread(&newU, sizeof(User), 1, fp)) {
            if (strcmp(newU.username, item.sender) == 0) {
                strcpy(newU.password, item.content);
                fseek(fp, -(long)sizeof(User), SEEK_CUR);
                fwrite(&newU, sizeof(User), 1, fp);
                break;
            }
        }
        fclose(fp);
    }

    else if(item.type == Appeal) {
        Student *p = findStudentById(item.sender);
        if (p) {
            printf("正在处理 %s 的成绩申诉\n", p->name);
            printf("选择要修正的科目: 1.C语言 2.数学 3.英语\n");
            int choice = readInt("请输入选项", 1, 3);

            if (choice == 1) {
                printf("原C语言成绩为: %.1f\n", p->score_c);
                p->score_c = readFloat("新C语言成绩: ", 0, 100);
            }
            else if (choice == 2) {
                printf("原数学成绩为: %.1f\n", p->score_math);
                p->score_math = readFloat("新数学成绩: ", 0, 100);
            }
            else if (choice == 3) {
                printf("原英语成绩为: %.1f\n", p->score_eng);
                p->score_eng = readFloat("新英语成绩: ", 0, 100);
            }
            calculateStats(p);

            printf("新的总分:%.1f\n", p->total);
            saveToBinaryFile();
        }
        else {
            printf("没找到id为%s的学生", item.sender);
        }
    }
}
}

//10. 菜单逻辑

//学生菜单
void studentMenu() {
    int choice;
    do {
        printf("\n======== 学生端 ========\n");
        printf("当前用户: %s\n", currentUser.username);
        printf("1. 查询个人信息\n");
        printf("2. 查看班级排名\n");
        printf("3. 查看成绩分布图\n");
        printf("4. 申请重置密码\n");
        printf("5. 成绩申诉\n");
        printf("0. 注销登录\n");
        choice = readInt("请选择: ", 0, 5);
        switch(choice) {
            case 1: {
                Student *myInfo = findStudentById(currentUser.id);
                if (myInfo) {
                    printTableHeader();
                    printStudentRow(myInfo);
                }else {
                    printf("系统里还没录入您的档案，请联系老师录入\n(当前绑定ID: %s)\n", currentUser.id);
                }
                pauseSystem();
                break;
            }
            case 2: {
                int option;
                option = readInt("输入科目(1.总分 2.C语言 3.数学 4.英语)", 1, 4);
                sortStudents(option, 2);
                viewWithPagination();
                break;
            }
            case 3: 
                performStatistics();
                break;
            case 4: {
                char pwd[MAX_PWD_LEN];
                printf("请输入新密码:");
                readString(pwd, MAX_PWD_LEN);
                
                addToDoItem(PwdReset, currentUser.username, pwd);
                break;
            }
            case 5: {
                char reason[100];
                printf("输入您的成绩申诉理由 :\n");
                readString(reason, 100);

                addToDoItem(Appeal, currentUser.id, reason);

                break;
            }
            case 0: break;
        }
    }while (choice != 0);
}

//教师菜单
void teacherMenu() {
    int choice;
    do {
       printf("\n======== 教师端 ========\n");
        printf("1. 录入新学生\n");
        printf("2. 浏览学生列表 (支持翻页)\n");
        printf("3. 查找学生\n");
        printf("4. 修改学生信息\n");
        printf("5. 删除学生\n");
        printf("6. 成绩排序\n");
        printf("7. 统计分析\n");
        printf("0. 注销登录\n");
        choice = readInt("请选择: ", 0, 7);

        switch (choice) {
            case 1: 
                addNewStudent();
                break;
            case 2:
                viewWithPagination();
                break;
            case 3:
                searchMenu();
                break;
            case 4: 
                modifyStudentMenu();
                break;
            case 5: {
                char id[MAX_ID_LEN];
                printf("输入要删除的学号: ");
                readString(id, MAX_ID_LEN);
                if (deleteStudentNode(id)) {
                    printf("删除成功\n");
                    saveToBinaryFile();
                }else {
                    printf("删除失败,没找到该学号\n");
                }
                pauseSystem();
                break;
            }
            case 6:
                menuSort();
                break;
            case 7:
                performStatistics();
                break;
            case 0: break;
        }
    }while (choice != 0);
}

//管理员菜单
void adminMenu() {
    int choice;
    do {
        printf("\n======== 管理员后台 ========\n");
        printf("1. 添加教师账号\n");
        printf("2. 添加学生账号\n");
        printf("3. 查看所有账号\n");
        printf("4. 进入教师模式 (管理数据)\n");
        printf("5. 清空所有数据 (慎用)\n");
        printf("6. 处理待办事项\n");
        printf("7. 强制修改账号密码\n");
        printf("8. 删除账号\n");
        printf("9. 导出所有账号密码\n");
        printf("0. 注销登录\n");
        choice = readInt("请选择: ", 0, 9);

        switch (choice) {
            case 1: 
                registerUser(2); 
                break;
            case 2: 
                registerUser(1); 
                break;
            case 3: 
                listAllUsers(); 
                break;
            case 4: 
                teacherMenu(); 
                break; 
            case 5: {
                printf("这将删除所有学生数据！确认吗 (y/n): ");
                char confirm[5]; readString(confirm, 5);
                if (confirm[0] == 'y') {
                    freeAllStudents();
                    remove(DATA_FILE);
                    printf("数据已重置。\n");
                }
                break;
            }
            case 6: {
                int id;
                int choice;
                do {
                    system("clear");
                    viewToDoItems();

                    id = readInt("请输入想要处理的申请流水号(输入0返回):", 0, 999);

                    if (id == 0) break;

                    choice = readInt("请审核(1.通过, 2.拒绝):", 1, 2);

                    updateToDoItem(id, choice);

                    printf("处理完毕,按回车继续\n");
                    pauseSystem();
                }while (1);
                break;
            }
            case 7: {
                char targetUser[30];
                char newPwd[MAX_PWD_LEN];
                printf("输入要重置密码的用户名:\n");
                readString(targetUser, 30);
                printf("输入新密码:\n");
                readString(newPwd, MAX_PWD_LEN);

                FILE *fp = fopen("users.dat", "rb+");
                if (fp == NULL) break;

                User u;
                int found = 0;
                while (fread(&u, sizeof(User), 1, fp)) {
                    if (strcmp(u.username, targetUser) == 0) {
                        found = 1;
                        strcpy(u.password, newPwd);

                        fseek(fp, -((long)sizeof(User)), SEEK_CUR);

                        fwrite(&u, sizeof(User), 1, fp);
                        printf("用户%s的密码已经修改\n", targetUser);
                        break;
                    }
                }
                if (found == 0) printf("未找到用户\n");
                fclose(fp);
                pauseSystem();
                break;
            }
            case 8: {
                char targetUser[30];
                printf("输入要删除的用户名\n");
                readString(targetUser, 30);
                //防止删除自己
                if (strcmp(targetUser, currentUser.username) == 0) {
                    printf("不能删除自己\n");
                    pauseSystem();
                    break;
                }

                FILE *fp = fopen("users.dat", "rb");
                FILE *tempFp = fopen("temp.dat", "wb");

                //安全判断
                if (fp == NULL || tempFp == NULL) {
                    printf("文件打开失败！\n");
                    if (fp) fclose(fp);
                    if (tempFp) fclose(tempFp);
                    pauseSystem();
                    break;
                }

                User u;
                int found = 0;

                while (fread(&u, sizeof(User), 1, fp)) {
                    if (strcmp(u.username, targetUser) != 0) {
                        fwrite(&u, sizeof(User), 1, tempFp);
                    }else {
                        found = 1;
                    }
                }
                fclose(fp);
                fclose(tempFp);

                if (found) {
                    remove("users.dat");//销毁旧文件
                    rename("temp.dat", "users.dat");
                    printf("用户%s已被删除\n", targetUser);
                }else {
                    remove("temp.dat");
                    printf("没找到\n");
                }
                pauseSystem();
                break;
            }
            case 9: {
                FILE *fp = fopen(USER_FILE, "rb");
                //打开一个新的csv文件
                FILE *exportFp = fopen("users_ex.csv", "w");
                if (!fp || !exportFp) {
                    printf("打开失败\n");
                    break;
                }

                //写入csv的表头
                fprintf(exportFp, "用户名,关联ID,密码,角色\n");

                User u;
                int cnt = 0;

                while (fread(&u, sizeof(User), 1, fp)) {
                    char roleName[10];
                    if (u.role == 1) strcpy(roleName, "学生");
                    else if(u.role == 2) strcpy(roleName, "老师");
                    else strcpy(roleName, "管理员");

                    fprintf(exportFp, "%s, %s, %s, %s\n", u.username, u.id, u.password, roleName);
                    cnt++;
                }
                fclose(fp);
                fclose(exportFp);
                printf("导出了%d个账号到users_ex.csv中\n", cnt);

                pauseSystem();
                break;
            }
            case 0: break;
        }
    } while (choice != 0);
}

int main() {
    // 1. 初始化系统
    initList();
       
    // 2. 加载数据
    loadFromBinaryFile();

    int mainChoice;
    while (1) {
        printf("\n########################################\n");
        printf("#       学生信息管理系统       #\n");
        printf("########################################\n");
        printf("1. 登录系统\n");
        printf("2. 注册账号 (默认注册为学生)\n");
        printf("0. 退出程序\n");
        
        mainChoice = readInt("请选择操作: ", 0, 2);

        if (mainChoice == 0) {
            printf("正在保存数据并退出...\n");
            saveToBinaryFile();
            freeAllStudents();
            free(head);
            printf("再见\n");
            break;
        }
        else if (mainChoice == 2) {
            registerUser(1);
        }
        else if (mainChoice == 1) {
            if (performLogin()) {
                printf("登录成功！欢迎, %s\n", currentUser.username);
                // 根据权限分发
                if (currentUser.role == 1) studentMenu();
                else if (currentUser.role == 2) teacherMenu();
                else if (currentUser.role == 3) adminMenu();
            } else {
                printf("登录失败：账号或密码错误。\n");
            }
        }
    }
    return 0;
}