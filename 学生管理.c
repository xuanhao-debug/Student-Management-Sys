#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 20
#define MAX_NAME_LEN 50
#define MAX_PWD_LEN 30
#define DATA_FILE "students.dat"   // 二进制数据文件
#define USER_FILE "users.dat"      // 用户账号文件
#define PAGE_SIZE 10               // 每页显示行数

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
    // 备份a的数据
    strcpy(temp.id, a->id);
    strcpy(temp.name, a->name);
    strcpy(temp.gender, a->gender);
    temp.age = a->age;
    temp.score_c = a->score_c;
    temp.score_math = a->score_math;
    temp.score_eng = a->score_eng;
    temp.total = a->total;
    temp.average = a->average;

    // a = b
    strcpy(a->id, b->id);
    strcpy(a->name, b->name);
    strcpy(a->gender, b->gender);
    a->age = b->age;
    a->score_c = b->score_c;
    a->score_math = b->score_math;
    a->score_eng = b->score_eng;
    a->total = b->total;
    a->average = b->average;

    // b = temp
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