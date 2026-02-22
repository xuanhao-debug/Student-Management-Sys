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
                    printf("输入错误: 数值必须在 %d 和 %d 之间\n", min, max);
                }
            }else {
                printf("输入错误: 请输入有效数字\n");
            }
        }
    }
}

void pauseStytem() {
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
    Student *p = head;
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

void freeAllList() {
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

    freeAllList();

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
    pauseStytem();
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
    if (head == NULL) {
        printf("还没有数据\n");
        pauseStytem();
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
                pauseStytem();
            }
        }else if (cmd[0] == 'p' || cmd[0] == 'P') {
            //cur指针必定是在每一页的开头，所以先检查前面有没有节点，有的话就肯定还有上一页
            if (cur->prev != head && cur->prev) {
                Student *temp = cur;
                int step = 0;
                while (temp->prev != head && step < PAGE_SIZE) {
                    temp = temp->next;
                    step++;
                }
                cur = temp;
                pageNumber--;
            }else {
                printf("已经是第一页了");
                pauseStytem();
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
    if (!head) {
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
        pauseStytem();
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
    pauseStytem();
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
        readString(newUser.username, MAX_NAME_LEN);
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
    pauseStytem();
}

//登陆
int performLogin() {
    char user[30], pwd[MAX_PWD_LEN];
    User u;
    FILE *fp = fopen(USER_FILE, "rb");

    if (!fp) {
        printf("检测到系统首次运行，正在创建默认管理员...\n");
        fp = fopen(USER_FILE, "wb");
        User admin = {"admin", "123456", "0000", 3};
        fwrite(&admin, sizeof(User), 1, fp);
        fp = fopen(USER_FILE, "rb");
        printf("默认账号:admin, 密码: 123456");
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
        else if(roleName == 2)
            strcpy(roleName, "教师");
        else 
            strcpy(roleName, "管理员");
        printf("%-20s %-10s\n", u.username, roleName);
    }
    fclose(fp);
    pauseStytem();
}










int main() {
    // 1. 初始化系统
    initList();
       
    // 2. 加载数据
    loadFromBinaryFile();

    int mainChoice;
    while (1) {
        printf("\n########################################\n");
        printf("#       学生信息管理系统 V2.1      #\n");
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
            
        }
        else if (mainChoice == 1) {
            if (performLogin()) {
                printf("登录成功！欢迎, %s\n", currentUser.username);
                // 根据权限分发
                if (currentUser.role == 1) 
                else if (currentUser.role == 2) 
                else if (currentUser.role == 3) 
            } else {
                printf("登录失败：账号或密码错误。\n");
            }
        }
    }

    return 0;
}