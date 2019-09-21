#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <stack>
#include <iomanip>

#define HANG_NUM 121//�����������
#define AC_LIE 38//Action������ ���ռ�������
#define GO_LIE 16//Goto������ ��״̬��
#define MAX_NUM 999//Action �� Goto���е�ʧ�ܷ��ţ�һ����ȡ��������žʹ����﷨����ʧ����
#define KEY_NUM 11//�ؼ���

using namespace std;

struct Expression{  //���ʽ
    string left;
    int left_num;
    string right[15];
    int num;
    Expression(){
        num = 0;
    }
};
struct Word_code {  //���뵥��
    string value;
    int num;
    bool ts;
};
Expression exp[100];//���ڴ�ű��ʽ������
int exp_num = 0;  //���ʽ��Ŀ

struct Sym_tab{    //���ű��е�һ����Ϣ
    string id;
    int type;       //0->char 1->int 2->double
    int width;
    int offset;		//����ƫ����,�������
    bool isarray;   //�ж��Ƿ�������
    string inside;
    Sym_tab(){
        inside = "";
    }
};
Sym_tab stable[100];
Sym_tab Typ;   //��ʱ�����
int off_num = 0;   //��ǰƫ����
int stnum = 0;   //���ű�����
string gra_word[30];   //��ʱ����

int temp_num = 0;    //��ʱ������Ŀ
int str_num = 0;    //�ַ���������Ŀ
int fag_num = 0;    //��ת�����Ŀ

struct Four_ex{   //��Ԫʽ
    string op;		//������
    string arg1;
    string arg2;
    string result;
    string fag;
    Four_ex(){
        fag = "";
    }
};
Four_ex fex[500];
int fex_num = 0;    //��Ԫʽ��Ŀ
struct NUM{
    string addr;
    int type;
    string off;
};
NUM tnumber;   //��ʱ����
NUM NA[50];
int na_num=0;
NUM cc;
NUM T[50];//�˳�ʽ��
int T_n = 0;
NUM F[50];//���֣���ʶ��ʽ��
int F_n = 0;
NUM E[50];//�Ӽ�ʽ��
int E_n = 0;
NUM AR;

string ROP = "";
int Bfalse[50];
int Btrue[50];
int Bnum = -1;
int Slist[50];

struct Judge{  //�������ʽ
    string E1;
    string ROP;
    string E2;
};
Judge B;

int action[HANG_NUM][AC_LIE];
int go_to[HANG_NUM][GO_LIE];
stack<int> state_stack;
stack<Word_code> word_stack;

string acode[500];
string adata[50];
int cnum = 0;
int dnum = 0;

string keyword[KEY_NUM] = {"main", "char", "int", "double", "if", "else", "while", "read", "write", "func", "endl"};

int keyjudge (string word);  //�жϱ�����
void exp_in ();  //���ʽ����
void table_in ();  //���������
void gra_analy ();  //�﷨����+�������
void word_analy();  //�ʷ�����
void sema_out ();  //�������������


int Code_pos = 0;  //�������λ��


void exp_in (){
    ifstream input;
    input.open("grammar/grammar.txt");//���ķ��ļ�
    string str;
    int num;
    while (input >> num >> str){
        exp[exp_num].left_num = num;//��ȡ�ķ�����ߵ����ֱ��
        exp[exp_num].left = str;//��ȡ�ķ���ߵķ��ռ�����
        string left = str;
        while (input >> str){
            if (str == "->") continue;
            if (str == "|"){
                exp_num++;
                exp[exp_num].left = left;
                exp[exp_num].left_num = num;
                continue;
            }
            if (str == "@"){
                exp_num++;
                break;
            }
            exp[exp_num].right[exp[exp_num].num++] = str;//���δ���ÿ���ķ������ʽ������
        }
    }
    input.close();
}
void table_in (){
    ifstream input;
    input.open("grammar/action.txt");
    for (int i = 0;i < HANG_NUM;i ++)
        for (int j = 0;j < AC_LIE;j ++)
            input >> action[i][j];
    input.close();
    input.open("grammar/goto.txt");
    for (int i = 0;i < HANG_NUM;i ++)
        for (int j = 0;j < GO_LIE;j ++)
            input >> go_to[i][j];
    input.close();
}
int find_id (string name){//���ұ����Ƿ��ڷ��ű��У�����ڷ��ص�ַ,���򷵻ص�ַ-1
    for (int i = 0;i < stnum;i++)
        if (name == stable[i].id) return i;
    return -1;
}
string new_temp(){//�����µı���#0,#1,#2,#3.....
    char temp[3];
    itoa(temp_num,temp,10);
    string s(temp);
    string ss = "#" + s;
    temp_num++;
    return ss;
}
string new_string(){//�����µ��ַ���string0 string1.....
    char temp[3];
    itoa(str_num,temp,10);//ת�����ַ���
    string s(temp);
    string ss = "string" + s;
    str_num++;
    return ss;
}
string new_fag(){//�����µ�
    char temp[3];
    itoa(fag_num,temp,10);
    string s(temp);
    string ss = "LC" + s;
    fag_num++;
    return ss;
}
int keyjudge (string word){
    for (int i = 0;i < KEY_NUM;i++)
        if (keyword[i] == word) return i;
    return KEY_NUM +2;
}
void gra_analy (){
    state_stack.push(0);//0״̬��stack
    ifstream in;
    ofstream out;
    in.open("word/word_res.txt");
    out.open("grammar/gra_res.txt");
    int num;
    string word;
    in >> num;//main��������
    if (num == 12){//����ǳ����ַ��� ��������ַ���
        char cstr[100];
        in.get();
        in.getline(cstr,99,'\n');//һ����ȫ������һ��������
        word = string(cstr);
    }
    else in >> word; 
    while (true){
        if (word == "else"){
            Slist[Bnum] = fex_num;
            fex[fex_num].op = "j";
            fex[fex_num].arg1 = "";
            fex[fex_num].arg2 = "";
            fex_num++;
        }
        Word_code w1; 
        int top_s = state_stack.top();
        int next = action[top_s][num];
        if (next == MAX_NUM){
            cout << top_s << " " << num << endl;
            cout << "ac_fail!" << endl;
            break;
        }
        else if (next == 0){
            cout << "grammar analy success!" << endl;
            break;
        }
        else if (next > 0){
            w1.num = num;
            w1.ts = true; 
            w1.value = word;//���ζ�ȡ�ʷ����������Ԫʽ���Ұ벿��
            state_stack.push(next);//״̬stack
            word_stack.push(w1);//����stack
            in >> num;
            if (num == 12){
                char cstr[100];
                in.get();
                in.getline(cstr,99,'\n');
                word = string(cstr);
            }
            else in >> word;
        }
        else{
            next = -next;
            string left = exp[next].left;
            int left_num = exp[next].left_num;
            out << left << " -> ";
            string tmp = " ";
            for(int i = 0;i < exp[next].num;i++){
                Word_code w2;
                w2 = word_stack.top();
                tmp = w2.value + " " + tmp;
                gra_word[exp[next].num-1-i] = w2.value;//������ֹ�Լ��͹�ԼȻ����Ӧ��״̬�ͷ��ų�ջ
                word_stack.pop(); 
                state_stack.pop();
            }
            out << tmp << endl;
            int index;
            string tt;
            string s;
            char temp[3];
            int cht;
            switch (next){//״̬��Ӧ���ķ���ǰ����gra_list����
                case 3:   //TN -> char
                    Typ.type = 0;//typ��ʱ���ű�
                    Typ.width = 1;
                    break;
                case 4:     //TN -> int
                    Typ.type = 1;
                    Typ.width = 2;
                    break;
                case 5:     //TN -> double
                    Typ.type = 2;
                    Typ.width = 8;
                    break;
                case 6:     //array -> id [ inum ]
                    index = find_id(gra_word[0]);
                    if (index == -1 ){
                        cout << "error:";
                        cout << gra_word[0] << "    not defined!" << endl;
                    }
                    else if (!stable[index].isarray){
                        cout << "error:";
                        cout << gra_word[0] << "    not array id!" << endl;
                    }
                    else{
                        tt = new_temp();
                        string ty[3]={"1","2","8"};//�ֽ��� char 1  int 2 double 8  
                        fex[fex_num].op = "*";
                        fex[fex_num].arg1 = gra_word[2];;
                        fex[fex_num].arg2 = ty[stable[index].type];
                        fex[fex_num].result = tt;
                        fex_num++;
                        AR.addr = gra_word[0];
                        AR.off = tt;
                        AR.type = stable[index].type;
                    }
                    break;
                case 7:     //array -> id [ id ]
                    index = find_id(gra_word[2]);
                    if (index == -1 ){
                        cout << "error:";
                        cout << gra_word[2] << "    not defined!" << endl;
                    }
                    else{
                        index = find_id(gra_word[0]);
                        if (index == -1 ){
                            cout << "error:";
                            cout << gra_word[0] << "    not defined!" << endl;
                        }
                        else if (!stable[index].isarray){
                            cout << "error:";
                            cout << gra_word[0] << "    not array id!" << endl;
                        }
                        else{
                            tt = new_temp();
                            string ty[3]={"1","2","8"}; //�ֽ��� char 1  int 2 double 8  
                            fex[fex_num].op = "*";
                            fex[fex_num].arg1 = gra_word[2];;
                            fex[fex_num].arg2 = ty[stable[index].type];
                            fex[fex_num].result = tt;
                            fex_num++;
                            AR.addr = gra_word[0];
                            AR.off = tt;
                            AR.type = stable[index].type;
                        }
                    }
                    break;
                case 8:     //num -> inum
                    tnumber.addr = gra_word[0];
                    tnumber.type = 1;
                    break;
                case 9:     //num -> dnum
                    tnumber.addr = gra_word[0];
                    tnumber.type = 2;
                    break;
                case 10:       //NA -> id
                    NA[na_num].addr = gra_word[0];
                    NA[na_num].off = "";
                    index = find_id(NA[na_num].addr);
                    if (index == -1){
                        cout << "error:";
                        cout << NA[na_num].addr << "    not defined!" << endl;
                    }
                    else{
                        NA[na_num].type = stable[index].type;
                    }
                    na_num++;
                    break;
                case 11:        //NA -> array
                    NA[na_num] = AR;
                    na_num++;
                    break;
                case 15:         //P -> TN id ;
                    Typ.offset = off_num ;
                    off_num +=  Typ.width;
                    Typ.id = gra_word[1];
                    Typ.isarray = false;
                    if (find_id(Typ.id) == -1)stable[stnum++] = Typ;
                    else {
                        cout << "error:";
                        cout << Typ.id << " multi-defined!" << endl;
                    }
                    break;
                case 16:        //P -> TN id [ num ] ;
                    if (tnumber.type == 2){
                        cout << "error:";
                        cout << gra_word[1] << "    array define error!" << endl;
                    }
                    else{
                        cht = atoi(tnumber.addr.c_str());
                        Typ.isarray = true;
                        Typ.offset = off_num ;
                        Typ.width = cht*Typ.width;
                        off_num += Typ.width;
                        Typ.id = gra_word[1];
                        if (find_id(Typ.id) == -1){
                            stable[stnum++] = Typ;
                        }
                        else {
                            cout << "error:";
                            cout << Typ.id << " multi-defined!" << endl;
                        }
                    }
                    break;
                case 17:    //TN id = ' c ' ;
                    Typ.offset = off_num ;
                    off_num +=  Typ.width;
                    Typ.id = gra_word[1];
                    Typ.isarray = false;
                    if (find_id(Typ.id) == -1){
                        stable[stnum++] = Typ;
                        fex[fex_num].op = "=";
                        fex[fex_num].arg1 = Typ.id;
                        fex[fex_num].arg2 = "";
                        cht =  gra_word[4][0];
                        itoa(cht,temp,10);
                        s = string (temp);
                        fex[fex_num].result = s;
                        fex_num++;
                    }
                    else {
                        cout << "error:";
                        cout << Typ.id << " multi-defined!" << endl;
                    }
                    break;
                case 18:    //TN id = E ;
                    E_n--;
                    Typ.offset = off_num ;
                    off_num += Typ.width;
                    Typ.id = gra_word[1];
                    Typ.isarray = false;
                    if (find_id(Typ.id) == -1){
                        stable[stnum++] = Typ;
                        fex[fex_num].op = "=";
                        fex[fex_num].arg1 = E[E_n].addr;
                        fex[fex_num].arg2 = "";
                        fex[fex_num].result = Typ.id;
                        fex_num++;
                    }
                    else {
                        cout << "error:";
                        cout << Typ.id << " multi-defined!" << endl;
                    }
                    break;
                case 19:    //P -> NA = E ;
                    E_n--;
                    na_num--;
                    temp_num = 0;
                    if (NA[na_num].type < E[E_n].type){
                        cout << "error:";
                        cout << NA[na_num].addr << "    type error!" << endl;
                    }
                    else {
                        if (NA[na_num].off != ""){
                            fex[fex_num].op = "[]=";
                            fex[fex_num].arg1 = NA[na_num].addr;
                            fex[fex_num].arg2 = NA[na_num].off;
                            fex[fex_num].result = E[E_n].addr;
                            fex_num++;
                        }
                        else{
                            fex[fex_num].op = "=";
                            fex[fex_num].arg1 = E[E_n].addr;
                            fex[fex_num].arg2 = "";
                            fex[fex_num].result = NA[na_num].addr;
                            fex_num++;
                        }
                    }
                    break;
                case 20:        //P -> NA = ' c ' ;
                    na_num--;
                    cc.addr = gra_word[3];
                    cc.type = 0;
                    index = find_id(NA[na_num].addr);
                    if (NA[na_num].type != cc.type){
                        cout << "error:";
                        cout << NA[na_num].addr << "    type error!" << endl;
                    }
                    else {
                         if (NA[na_num].off != ""){
                            fex[fex_num].op = "[]=";
                            cht = cc.addr[0];
                            itoa(cht,temp,10);
                            s = string (temp);
                            fex[fex_num].arg1 = NA[na_num].addr;
                            fex[fex_num].arg2 = NA[na_num].off;
                            fex[fex_num].result = s;
                            fex_num++;
                        }
                        else{
                            fex[fex_num].op = "=";
                            cht = cc.addr[0];
                            itoa(cht,temp,10);
                            s = string (temp);
                            fex[fex_num].arg1 = s;
                            fex[fex_num].arg2 = "";
                            fex[fex_num].result = NA[na_num].addr;
                            fex_num++;
                        }
                    }
                    break;
                case 21:     //P -> while ( B ) { S }
                    fex[fex_num].op = "j";
                    fex[fex_num].arg1 = "";
                    fex[fex_num].arg2 = "";
                    itoa(Btrue[Bnum],temp,10);
                    s = string (temp);
                    fex[fex_num].result = s;
                    fex_num++;

                    itoa(fex_num,temp,10);
                    s = string (temp);
                    fex[Bfalse[Bnum]].result = s;

                    Bnum--;
                    break;
                case 22:    //P -> read NA ;
                    na_num--;
                    fex[fex_num].op = "r";
                    fex[fex_num].arg1 = NA[na_num].addr;
                    if (NA[na_num].off == "-1"){
                        fex[fex_num].arg2 = "";
                    }
                    else fex[fex_num].arg2 = NA[na_num].off;
                    fex[fex_num].result = "";
                    fex_num++;
                    temp_num = 0;
                    break;
                case 23:    //P -> write NA ;
                    na_num--;
                    fex[fex_num].op = "w";
                    fex[fex_num].arg1 = NA[na_num].addr;
                    if (NA[na_num].off == "-1"){
                        fex[fex_num].arg2 = "";
                    }
                    else fex[fex_num].arg2 = NA[na_num].off;
                    fex[fex_num].result = "";
                    fex_num++;
                    temp_num = 0;
                    break;
                case 24:    //P -> write " str " ;
                    stable[stnum].id = new_string();
                    stable[stnum].inside = gra_word[2];
                    stable[stnum].type = 0;
                    stable[stnum].isarray = 1;
                    stable[stnum].offset = off_num;
                    stable[stnum].width = gra_word[2].size();
                    off_num += stable[stnum].width;
                    stnum++;

                    fex[fex_num].op = "w";
                    fex[fex_num].arg1 = stable[stnum-1].id;
                    fex[fex_num].arg2 = "str";
                    fex[fex_num].result = "";
                    fex_num++;
                    break;
                case 25:    //P -> write endl ;
                    fex[fex_num].op = "w";
                    fex[fex_num].arg1 = "endl";
                    fex[fex_num].arg2 = "";
                    fex[fex_num].result = "";
                    fex_num++;
                    break;
                case 26:     //P -> if ( B ) { S }
                    itoa(fex_num,temp,10);
                    s = string (temp);
                    fex[Bfalse[Bnum]].result = s;
                    Bnum--;
                    break;
                case 27:     //P -> if ( B ) { S } else { S }
                    itoa(fex_num,temp,10);
                    s = string (temp);
                    fex[Slist[Bnum]].result = s;

                    itoa(Slist[Bnum]+1,temp,10);
                    s = string (temp);
                    fex[Bfalse[Bnum]].result = s;
                    Bnum--;
                    break;
                case 28:    //E -> E + T
                    T_n--;
                    E_n--;
                    if (E[E_n].addr[0] == '#') tt = E[E_n].addr;
                    else if (T[T_n].addr[0] == '#') tt = T[T_n].addr;
                    else tt = new_temp();
                    index = (E[E_n].type > T[T_n].type) ? E[E_n].type : T[T_n].type;
                    fex[fex_num].op = "+";
                    fex[fex_num].arg1 = E[E_n].addr;
                    fex[fex_num].arg2 = T[T_n].addr;
                    fex[fex_num].result = tt;
                    fex_num++;
                    E[E_n].addr = tt;
                    E[E_n].type = index;
                    E_n++;
                    break;
                case 29:    //E -> E - T
                    T_n--;
                    E_n--;
                    if (E[E_n].addr[0] == '#') tt = E[E_n].addr;
                    else if (T[T_n].addr[0] == '#') tt = T[T_n].addr;
                    else tt = new_temp();
                    index = (E[E_n].type > T[T_n].type) ? E[E_n].type : T[T_n].type;
                    fex[fex_num].op = "-";
                    fex[fex_num].arg1 = E[E_n].addr;
                    fex[fex_num].arg2 = T[T_n].addr;
                    fex[fex_num].result = tt;
                    fex_num++;
                    E[E_n].addr = tt;
                    E[E_n].type = index;
                    E_n++;
                    break;
                case 30:    //E -> T
                    T_n--;
                    E[E_n].addr = T[T_n].addr;
                    E[E_n].type = T[T_n].type;
                    E_n++;
                    break;
                case 31:    //T -> T * F
                    F_n--;
                    T_n--;
                    if (T[T_n].addr[0] == '#' && F[F_n].addr[0] == '#'){
                        tt = (T[T_n].addr[1] < F[F_n].addr[1]) ? T[T_n].addr : F[F_n].addr;
                    }
                    else if (T[T_n].addr[0] == '#') tt = T[T_n].addr;
                    else if (F[F_n].addr[0] == '#') tt = F[F_n].addr;
                    else tt = new_temp();
                    index = (F[F_n].type > T[T_n].type) ? F[F_n].type : T[T_n].type;
                    fex[fex_num].op = "*";
                    fex[fex_num].arg1 = T[T_n].addr;
                    fex[fex_num].arg2 = F[F_n].addr;
                    fex[fex_num].result = tt;
                    fex_num++;
                    T[T_n].addr = tt;
                    T[T_n].type = index;
                    T_n++;
                    break;
                case 32:    //T -> T / F
                    F_n--;
                    T_n--;
                    if (T[T_n].addr[0] == '#' && F[F_n].addr[0] == '#'){
                        tt = (T[T_n].addr[1] < F[F_n].addr[1]) ? T[T_n].addr : F[F_n].addr;
                    }
                    else if (T[T_n].addr[0] == '#') tt = T[T_n].addr;
                    else if (F[F_n].addr[0] == '#') tt = F[F_n].addr;
                    else tt = new_temp();
                    index = (F[F_n].type > T[T_n].type) ? F[F_n].type : T[T_n].type;
                    fex[fex_num].op = "/";
                    fex[fex_num].arg1 = T[T_n].addr;
                    fex[fex_num].arg2 = F[F_n].addr;
                    fex[fex_num].result = tt;
                    fex_num++;
                    T[T_n].addr = tt;
                    T[T_n].type = index;
                    T_n++;
                    break;
                case 33:    //T -> F
                    F_n--;
                    T[T_n].addr = F[F_n].addr;
                    T[T_n].type = F[F_n].type;
                    T_n++;
                    break;
                case 34:    //F -> ( E )
                    E_n--;
                    F[F_n].addr = E[E_n].addr;
                    F[F_n].type = E[E_n].type;
                    F_n++;
                    break;
                case 35:    //F -> NA
                    na_num--;
                    if (NA[na_num].off != ""){
                        fex[fex_num].op = "=[]";
                        fex[fex_num].arg1 = NA[na_num].addr;
                        fex[fex_num].arg2 = NA[na_num].off;
                        tt = new_temp();
                        fex[fex_num].result = tt;
                        NA[na_num].addr = tt;
                        NA[na_num].off = "";
                        fex_num++;
                    }
                    F[F_n].addr = NA[na_num].addr;
                    F[F_n].type = NA[na_num].type;
                    F_n++;
                    break;
                case 36:    //F -> num
                    F[F_n].addr = tnumber.addr;
                    F[F_n].type = tnumber.type;
                    F_n++;
                    break;
                case 37:     //B -> E ROP E
                    E_n--;
                    B.E2 = E[E_n].addr;
                    B.ROP = ROP;
                    ROP = "";
                    Bnum++;
                    Btrue[Bnum] = fex_num;
                    Bfalse[Bnum] = fex_num+1;
                    fex[fex_num].op = "j" + B.ROP;
                    fex[fex_num].arg1 = B.E1;
                    fex[fex_num].arg2 = B.E2;
                    itoa(fex_num+2,temp,10);
                    s = string (temp);
                    fex[fex_num].result = s;
                    fex_num++;
                    fex[fex_num].op = "j";
                    fex[fex_num].arg1 = "";
                    fex[fex_num].arg2 = "";
                    fex_num++;
                    E_n++;
                    temp_num = 0;
                    break;
                case 38:     //ROP -> >
                    B.E1 = E[E_n-1].addr;
                    ROP = gra_word[0];
                    break;
                case 39:     //ROP -> <
                    B.E1 = E[E_n-1].addr;
                    ROP = gra_word[0];
                    break;
                case 40:     //ROP -> >=
                    B.E1 = E[E_n-1].addr;
                    ROP = gra_word[0];
                    break;
                case 41:     //ROP -> <=
                    B.E1 = E[E_n-1].addr;
                    ROP = gra_word[0];
                    break;
                case 42:     //ROP -> ==
                    B.E1 = E[E_n-1].addr;
                    ROP = gra_word[0];
                    break;
                case 43:     //ROP -> !=
                    B.E1 = E[E_n-1].addr;
                    ROP = gra_word[0];
                    break;
            }
            w1.num = left_num;
            w1.value = left;
            w1.ts = false;
            word_stack.push(w1);
            top_s = state_stack.top();
            next = go_to[top_s][left_num];
            if (next == MAX_NUM){
                cout << top_s << " " << left_num << endl;
                cout << "go_fail!" << endl;
                break;
            }
            else{
                state_stack.push(next);
            }
        }
    }
    in.close();
    out.close();
}
void word_analy(){
    fstream readin;
    ofstream output;
    output.open("word/word_res.txt");
    string word;
    bool eof = false;//�Ƿ��ļ�β
    bool ready = true;
    bool dot = false;//�Ƿ��⵽С����
    int pos,kcn;
    char ch;
    char tword[50];
	readin.open ("�����ļ�3.txt",ios::in);
    while (!readin.eof()){
        if (ready)
		{
			readin.get(ch);
		}
        else ready = true;
        if (ch == 32 || ch == 10 || ch == 13 || ch ==9) continue;//�����ո���߻��з��ȹ��˷� ����
        if ((ch >= 'a' && ch <= 'z')|| (ch >= 'A' && ch <= 'Z')) {
            pos = 0;
            tword[pos++] = ch;
            while (readin.get(ch)){
                if ((ch >= 'a' && ch <= 'z')|| (ch >= 'X' && ch <= 'Z') || (ch >= '0' && ch <= '9')){
                    if (pos < 49) tword[pos++] = ch;
                    if (readin.eof()) eof = true;
                }
                else break;
            }
            tword[pos] = 0;
            word = string(tword);
            kcn = keyjudge(word);//����ǹؼ��־ͷ��عؼ��ֵ������ַ��������ǵĻ��ͷ��ر�ʶ��
            output << setw(10) << kcn << " " << setw(10) << word << endl;
            if (eof == true) break;
            ready = false;
        }
        else if (ch >= '0' && ch <= '9'){
            dot = false;
            pos = 0;
            tword[pos++] = ch;
            while (readin.get(ch)){
                if (ch >= '0' && ch <= '9'){
                    if (pos < 49) tword[pos++] = ch;
                    if (readin.eof()) eof = true;
                }
                else if (ch == '.'){
                    if (!dot) {
                        if (pos < 49) tword[pos++] = ch;
                        if (readin.eof()) eof = true;
                        dot = true;
                    }
                    else break;
                }
                else break;
            }
            tword[pos] = 0;
            word = string(tword);
            kcn = KEY_NUM +3;//������
            if (dot) kcn = KEY_NUM +4;//������
            output << setw(10) << kcn << " " << setw(10) << word << endl;
            if (eof == true) break;
            ready = false;
        }
        else if (ch == '\''){//���ch�ǵ�����
            kcn = KEY_NUM +23;
            output << setw(10) << kcn << " " << setw(10) << ch << endl;
            readin.get(ch);
            if (ch != '\'' && ch != '\"'){
                kcn = KEY_NUM +0;//�����ַ�
                output << setw(10) << kcn << " " << setw(10) << ch << endl;
                readin.get(ch);
                if (ch == '\'') {
                    kcn = KEY_NUM +23;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                }
                else {
                    kcn = -1;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                }
            }
            else if (ch == '\''){
                kcn = KEY_NUM +23;
                output << setw(10) << kcn << " " << setw(10) << ch << endl;
            }
            else{
                kcn = -1;
                output << setw(10) << kcn << " " << setw(10) << ch << endl;
            }
        }
        else if (ch == '\"'){//˫����
            pos = 0;
            kcn = KEY_NUM +24;//�ַ���
            output << setw(10) << kcn << " " << setw(10) << ch << endl;
            while (readin.get(ch)){
                if (ch != '\'' && ch != '\"'){
                    if (pos < 49) tword[pos++] = ch;
                }
                else break;
            }
            tword[pos] = 0;
            word = string(tword);
            kcn = KEY_NUM +1;
            output << setw(10) << kcn << " " << setw(10) << word << endl;
            if (ch == '\"'){
                kcn = KEY_NUM +24;
                output << setw(10) << kcn << " " << setw(10) << ch << endl;
            }
            else {
                kcn = -1;
                output << setw(10) << kcn << " " << setw(10) << ch << endl;
            }
        }
        else{
            switch (ch){
                case '+':
                    kcn = KEY_NUM +11;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '-':
                    kcn = KEY_NUM +12;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '*':
                    kcn = KEY_NUM +14;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '/'://ʶ��ע��/**/�ͳ��š�/��
                    if (readin.eof()){
                        kcn = KEY_NUM +13;
                        output << setw(10) << kcn << " " << setw(10) << ch << endl;
                        eof = true;
                        break;
                    }
                    readin.get(ch);
                    if (ch == '*'){
                        while (!readin.eof()){
                            readin.get(ch);
                            if (ch == '*'){
                                readin.get(ch);
                                if (ch == '/') break;
                            }
                        }
                    }
                    else{
                        kcn = KEY_NUM +13;
                        output << setw(10) <<  kcn << setw(10) << "/ " << endl;
                        ready = false;
                    }
                    break;
                case '=':
                    if (readin.eof()){
                        kcn = KEY_NUM +15;
                        output << setw(10) << kcn << " " << setw(10) << ch << endl;
                        eof = true;
                        break;
                    }
                    readin.get(ch);
                    if (ch == '='){
                        kcn = KEY_NUM +16;
                        output << setw(10) << kcn << setw(10) << "== " << endl;
                    }
                    else {
                      kcn = KEY_NUM +15;
                      output << setw(10) << kcn << " " << setw(10) << '=' << endl;
                      ready = false;
                    }
                    break;
                case ';':
                    kcn = KEY_NUM +25;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '>':
                    if (readin.eof()){
                        kcn = KEY_NUM +18;
                        output << setw(10) << kcn << " " << setw(10) << ch << endl;
                        eof = true;
                        break;
                    }
                    readin.get(ch);
                    if (ch == '=') {
                        kcn = KEY_NUM +20;
                        output << setw(10) << kcn << setw(10) << ">= " << endl;
                    }
                    else {
                        kcn = KEY_NUM +18;
                        output << setw(10) << kcn << " " << setw(10) << '>' << endl;
                        ready = false;
                    }
                    break;
                case '<':
                    if (readin.eof()){
                        kcn = KEY_NUM +19;
                        output << setw(10) << kcn << " " << setw(10) << ch << endl;
                        eof = true;
                        break;
                    }
                    readin.get(ch);
                    if (ch == '=') {
                        kcn = KEY_NUM +21;
                        output << setw(10) << kcn << setw(10) << "<= " << endl;
                    }
                    else {
                        kcn = KEY_NUM +19;
                        output << setw(10) << kcn << " " << setw(10) << '<' << endl;
                        ready = false;
                    }
                    break;
                case '!':
                    if (readin.eof()){
                        kcn = -1;
                        output << setw(10) << kcn << " " << setw(10) << ch << endl;
                        eof = true;
                        break;
                    }
                    readin.get(ch);
                    if (ch == '=') {
                        kcn = KEY_NUM +17;
                        output << setw(10) << kcn << setw(10) << "<= " << endl;
                    }
                    else {
                        kcn = -1;
                        output << setw(10) << kcn << " " << setw(10) << '!' << ch << endl;
                        ready = false;
                    }
                    break;
                case '(':
                    kcn = KEY_NUM +9;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case ')':
                    kcn = KEY_NUM +10;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '[':
                    kcn = KEY_NUM +7;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case ']':
                    kcn = KEY_NUM +8;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '{':
                    kcn = KEY_NUM +5;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case '}':
                    kcn = KEY_NUM +6;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                case ',':
                    kcn = KEY_NUM +22;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;
                    break;
                default:
                    kcn = -1;
                    output << setw(10) << kcn << " " << setw(10) << ch << endl;

            }
        if (eof == true) break;
        }
    }
    kcn = KEY_NUM +26;
    output << setw(10) << kcn << setw(10) << "# " << endl;//���������һ��#
    cout << "word analy success!" << endl;
    readin.close();
    output.close();
}
void sema_out (){
    ofstream out;
    out.open("sema/sema_res.txt");
    for (int i = 0;i<fex_num;i++){
        out << i << " ( ";
        out << fex[i].op << " , ";
        out << fex[i].arg1 << " , ";
        out << fex[i].arg2 << " , ";
        out << fex[i].result << " )";
        out << endl;
    }
    out.close();
    cout << "semantic analysis success!" << endl;
}
int main()
{
    word_analy();
    exp_in();
    table_in();
    gra_analy();
    sema_out();
	printf("���word.res.txt�ļ��鿴�ʷ��������\n");
	printf("���gra_res.txt�ļ��鿴�﷨�������\n");
    printf("���sema_res.txt�ļ��鿴��Ԫʽ���\n");
    return 0;
}




