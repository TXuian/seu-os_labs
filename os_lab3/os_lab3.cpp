#include<vector>
#include<stdio.h>
#include<random>
#include<time.h>
#include<iostream>
#include<windows.h>
#include<iomanip>

using std::vector;
using std::cin;
using std::cout;

#define N 7  //number of processes
#define M 10  //number of resources
#define MOD(x,y) if(0==(y)){(x)=0;}else{(x)=(int)(rand()%(y));} 

vector<int> Available(M);
vector<vector<int>> Max(N, vector<int>(M)), Allocation(N, vector<int>(M)), Need(N, vector<int>(M));

unsigned my_pid = 0;
HANDLE Mutex;
HANDLE AskForResources;

/*help functions*/

/* compare if v1 is less equal (not greater) than v2
*  @return: 1 for (v1<v2),
*          -1 for incomparable
*           0 for other condition */
template <class T>
int vec_compare_NotGreater(const vector<T>& v1, const vector<T>& v2) {
    if (v1.size() != v2.size()) { return -1; }  //different size
    size_t vec_size = v1.size();
    for (size_t i = 0; i < vec_size; ++i) {
        if (v1[i] > v2[i]) { return 0; }  /*false if v1[i]>v2[i]*/
    }
    return 1;
}

/* culculate v1=v1+v2
*  @return -1: different size of v1 and v2
*          0: normal return */
template <class T>
int vec_PlusEuqal(vector<T>& v1, const vector<T>& v2) {
    if (v1.size() != v2.size()) { return -1; }  /*incompatable*/
    size_t vec_size = v1.size();
    for (size_t i = 0; i < vec_size; ++i) {  /*chg all elements in v1*/
        v1[i] += v2[i];
    }
    return 0;
}

/* culculate v3=v1+v2
*  @return -1: different size of v1 and v2
*          0: normal return */
template <class T>
int vec_Plus(const vector<T>& v1, const vector<T>& v2, vector<T>& v3) {
    if (v1.size() != v2.size()) { return -1; }  /*incompatable*/
    size_t vec_size = v1.size();
    v3.resize(vec_size);
    for (size_t i = 0; i < vec_size; ++i) {  /*chg all elements in v1*/
        v3[i] = v1[i] + v2[i];
    }
    return 0;
}

/* culculate v1=v1-v2
*  @return -1: different size of v1 and v2
*          0: normal return */
template <class T>
int vec_MinusEqual(vector<T>& v1, const vector<T>& v2) {
    if (v1.size() != v2.size()) { return -1; }  /*incompatable*/
    size_t vec_size = v1.size();
    for (size_t i = 0; i < vec_size; ++i) {  /*chg all elements in v1*/
        v1[i] -= v2[i];
    }
    return 0;
}

/* culculate v3=v1-v2
*  @return -1: different size of v1 and v2
*          0: normal return */
template <class T>
int vec_Minus(const vector<T>& v1, const vector<T>& v2, vector<T>& v3) {
    if (v1.size() != v2.size()) { return -1; }  /*incompatable*/
    size_t vec_size = v1.size();
    v3.resize(vec_size);
    for (size_t i = 0; i < vec_size; ++i) {  /*chg all elements in v1*/
        v3[i] = v1[i] - v2[i];
    }
    return 0;
}

/* print matrix*/
template <class T>
void PrintMatrix(const vector<vector<T>>& mat) {
    for (auto x : mat) {
        for (T y : x) {
            cout << std::setw(4) << std::setiosflags(std::ios::left) << y;
        }
        printf("\n");
    }
}

/*print vector*/
template <class T>
void PrintVector(const vector<T>& vec) {
    for (T x : vec) {
        cout << std::setw(4) << std::setiosflags(std::ios::left) << x;
    }
    printf("\n");
}

/*print matrix and vectors of system*/
void PrintSystemState() {
    printf("Available vector:\n");
    PrintVector(Available);
    printf("Max matrix:\n");
    PrintMatrix(Max);
    printf("Allocation matrix:\n");
    PrintMatrix(Allocation);
    printf("Need matrix:\n");
    PrintMatrix(Need);
}

/*core part of banker algorithm*/

bool SecurityTest(const vector<int>& avai, const vector<int>& need, const vector<int>& alloc, size_t pid) {
    vector<int> Work(M); /*working resources vector*/
    vector<bool> Finish(N, false); /*finish list*/
    for (int i = 0; i < M; ++i) {
        Work[i] = avai[i];
    } //resources remain
    bool cnt = true;
    while (cnt) {
        cnt = false;
        for (int i = 0; i < N; ++i) {
            if (i == pid) {  /*pid case*/
                if (false == Finish[i] && 1 == vec_compare_NotGreater<int>(need, Work)) { /*found suitable process*/
                    if (-1 == vec_PlusEuqal(Work, alloc)) {  /*Work=Work+Allocation*/
                        throw("invalid change occured.");
                    }
                    cnt = true;
                    Finish[i] = true;
                    break; /*find from begining*/
                }
            }
            if (false == Finish[i] && 1 == vec_compare_NotGreater<int>(Need[i], Work)) { /*found suitable process*/
                if (-1 == vec_PlusEuqal(Work, Allocation[i])) {  /*Work=Work+Allocation*/
                    throw("invalid change occured.");
                }
                cnt = true;
                Finish[i] = true;
                break; /*find from begining*/
            }
        }
    }
    for (bool x : Finish) {  /*test if all elements in Finish is true*/
        if (false == x) { return false; }
    }
    return true;
}

bool RequestResource(unsigned pid, const vector<int>& Request) {
    /*check if pid is valid*/
    if (pid < 0 || pid>6) {
        printf("invalid process id.\nAllocation fail.\n");
        return false;
    }
    /*critical part*/
    WaitForSingleObject(Mutex, INFINITE);
    if (1 != vec_compare_NotGreater(Request, Need[pid])) {
        printf("process%d: error occured, process%d's request exceed his max request.\n", pid, pid);
        printf("Allocation fail.\n");
        ReleaseMutex(Mutex);
        return false;
    }
    while (1 != vec_compare_NotGreater(Request, Available)) {
        printf("process%d: No enough resources to allocate.\nAllocation fail.\n", pid);
        /*be hold that if the asked resources exceed the number of resources that system possessed,
        the process will wait forever*/
        ReleaseMutex(Mutex);
        WaitForSingleObject(AskForResources, INFINITE);
        WaitForSingleObject(Mutex, INFINITE);
    }
    /*security check and (if can), allocate*/
    vector<int> avai, alloc, need;
    /*simulated allocation*/
    vec_Minus<int>(Available, Request, avai);
    vec_Plus<int>(Allocation[pid], Request, alloc);
    vec_Minus<int>(Need[pid], Request, need);
    if (SecurityTest(avai, need, alloc, pid)) {  /*security varified*/
        /*true allocation*/
        vec_MinusEqual<int>(Available, Request);
        vec_PlusEuqal<int>(Allocation[pid], Request);
        vec_MinusEqual<int>(Need[pid], Request);
        printf("process%d allocate success!\n", pid);
    }
    else {
        printf("process%d: unsafe application. Allocate fail!\n", pid);
    }
    /*print current matrix(vectors)*/
    /*printf("Request vector:\n");
    PrintVector<int>(Request);
    PrintSystemState();
    printf("\n");*/
    ReleaseMutex(Mutex);
    return true;
}

bool ReleaseResource(unsigned pid, const vector<int>& Release) {
    /*check if pid is valid*/
    if (pid < 0 || pid>6) {
        printf("process%d: invalid process id.\nAllocation fail.\n", pid);
        return false;
        /*critical part*/
        WaitForSingleObject(Mutex, INFINITE);
    }if (1 != vec_compare_NotGreater(Release, Allocation[pid])) {
        printf("process%d: error occured, process%d wants to release resources more that he possessed\n", pid, pid);
        ReleaseMutex(Mutex);
        return false;
    }
    /*release resources*/
    vec_PlusEuqal<int>(Available, Release);
    vec_MinusEqual<int>(Allocation[pid], Release);
    vec_PlusEuqal<int>(Need[pid], Release);
    ReleaseSemaphore(AskForResources, 1, NULL);
    printf("process%d release successful.\n", pid);
    /*print current matrix(vectors)*/
    /*printf("Release vector:\n");
    PrintVector<int>(Release);
    PrintSystemState();
    printf("\n");*/
    ReleaseMutex(Mutex);
    return true;
}

/*prehandle to main*/

/* initiate a vector with either random number or by user input
* @param vec: vector that need initiation
* @param vname: vector's name that need to info user
* @param uinput: whether user need to input or not
*/
template <class T>
void InitSimilationVector(vector<T>& vec, const char* vname, bool uinput) {
    size_t v_size = vec.size();
    /*init type 1: input by users*/
    if (uinput) {
        printf("Please input your %s vector (with row %d):\n", vname, v_size);
        for (size_t i = 0; i < v_size; ++i) {
            cin >> vec[i];
        }
    }
    else { /*init type 2: default setting (for test conveniency)*/
        for (size_t i = 0; i < v_size; ++i) {
            vec[i] = (T)((int)rand() % 30);  /*randomly chose a number*/
        }
        printf("the %s vector is:\n", vname);
        for (auto x : vec) {
            std::cout << x << ' ';
        }
        printf("\n");
    }
}

/* initiate a matrix with either random number or by user input
* @param mat: matrix that need initiation
* @param mname: matrix's name that need to info user
* @param uinput: whether user need to input or not
*/
template <class T>
void InitSimulationMatrix(vector<vector<T>>& mat, const char* mname, bool uinput) {
    /*init type 1: input by users*/
    if (uinput) {
        printf("Please input your %s matrix (with row %d and column %d):\n", mname, N, M);
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) { cin >> mat[i][j]; }
        }
    }
    else { /*init type 2: default setting (for test conveniency)*/
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                MOD(mat[i][j], Available[j]);
            }
        }
        printf("the %s matrix is:\n", mname);
        PrintMatrix<int>(mat);
    }
}

DWORD WINAPI Process(void* param) {
    int pid = my_pid++;
    vector<int> Request(M, 0);
    vector<int> Release(M, 0);
    Sleep(2000);
    while (TRUE) {
        srand((unsigned int)time(NULL));
        Sleep(rand() % 3000);
        if (0 == (int)(rand() % 2)) { //asking for resources
            /*generate Request*/
            for (int i = 0; i < M; ++i) {
                MOD(Request[i], Need[pid][i]);
            }
            RequestResource(pid, Request);
        }
        else {  //release resources
           /*generate release matrix*/
            for (int i = 0; i < M; ++i) {
                MOD(Release[i], Allocation[pid][i]);
            }
            ReleaseResource(pid, Release);
        }
    }
}

int main(int argc, char** argv) {
    /*init mutex for changing matrix(vectors)*/
    Mutex = CreateMutex(NULL, FALSE, NULL);
    AskForResources = CreateSemaphore(NULL, 1, 7, NULL);
    /*init resources matrix*/
    for (auto x : Allocation) {  /*init Allocation*/
        for (int y : x) { y = 0; }
    }
    /*available resources in system*/
    InitSimilationVector(Available, "Available", false);
    /*max number of resources that processes may require*/
    InitSimulationMatrix<int>(Max, "Max", false);
    /*number of resources that processes may still need*/
    Need = Max;

    /*allocate resources to process*/
    for (int i = 0; i < 7; ++i) { /*7 processes*/
        CreateThread(NULL, 0, Process, NULL, 0, NULL);
    }

    Sleep(40000);
    return 0;
}