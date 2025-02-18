#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sstream>
#include <string>
#include <cctype>

using namespace std;

class operation {
public:
    int loc;
    string label, operate, operand, opcode, operand_loc;
    bool operror=false;
    bool symerror=false;

public:
    operation(string loc, string label, string operate, string operand,string opcode) {
        this->loc = stoi(loc);
        this->label = label;
        this->operate = operate;
        this->operand = operand;
        this->opcode = opcode;
    }
    void result() {
        ofstream fout;
        fout.open("/Users/yangjaemin/Library/Mobile Documents/com~apple~CloudDocs/강의자료/시스템소프트웨어/2023111938_양재민_10주차/newpro/result.txt", ios::app); // append mode
        if (fout.is_open()) {
            
            if(operror ==false){
                fout <<hex<< "Loc : "<< loc<< dec << ", " <<"Label : "<< label << ", " <<"Operation : "<< operate << ", " <<"Operand : "<< operand<< ", Opcode : "<<opcode << operand_loc << endl;
                if(symerror==true) fout<<"**undefined symbol**"<<endl;
            }
            else{
                fout <<hex<< "Loc : "<< loc<< dec << ", " <<"Label : "<< label << ", " <<"Operation : "<< operate << ", " <<"Operand : "<< operand <<", Opcode : "<<endl;
                if(operror==true&& this->operate!="start"&&this->operate!="end") fout << "**undefined operation code**"<<endl;
                if(symerror==true&& this->operate!="start"&&this->operate!="end") fout <<"**undefined symbol**"<<endl;
            }
            fout.close();
        } else {
            cerr << "파일 쓰기 오류" << endl;
        }
    }
};

int find_operand(string inp, vector<pair<string, int>> symtab){
    size_t index = inp.find(','); // 쉼표 위치 찾기
    string operand_name = (index != string::npos) ? inp.substr(0, index) : inp; // 쉼표 이전 문자열 추출
    
    for (int i = 0; i < symtab.size(); i++) {
        if (operand_name == symtab[i].first) {
            return symtab[i].second;
        }
    }
    return -1;
}
string find_opcode(string inp, vector<pair<string, string>> vec){//opcode 리턴
    for(int i=0;i<vec.size();i++){
        if(inp==vec[i].first){
            return vec[i].second;
        }
    }
    if(inp=="byte" || inp=="word"||inp=="resb"||inp=="resw"){
        return "";
    }
    return "**undefined operation code**";
}
string tosmaller(string inp){//opcode는 소문자로 비교한다
    for(int i = 0; i < inp.length(); i++){
        inp[i] = tolower(inp[i]);
    }
    return inp;
}
string stringToHex(const string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        oss << hex << setw(2) << setfill('0')<<uppercase << static_cast<int>(c);
    }
    return oss.str();
}
int main() {
    vector<operation> oper;
    vector<pair<string, string>> vec;
    vector<pair<string, int>> symtab;
    vector<string> src;
    string tmp, tmp1, opcode;
    ifstream fin;
    fin.open("/Users/yangjaemin/Library/Mobile Documents/com~apple~CloudDocs/강의자료/시스템소프트웨어/2023111938_양재민_10주차/newpro/optab.txt");
    if (!fin) {
        cerr << "optab.txt 파일 입력 오류" << endl;
        return 1; // 프로그램 종료
    }
    
    while (fin >> tmp >> tmp1) {
        tmp = tosmaller(tmp);
        vec.push_back(make_pair(tmp, tmp1)); // optab 읽기
    }
    fin.close();
    
    fin.open("/Users/yangjaemin/Library/Mobile Documents/com~apple~CloudDocs/강의자료/시스템소프트웨어/2023111938_양재민_10주차/newpro/SRCFILE"); // srcfile 읽기
    if (!fin) {
        cerr << "SRCFILE 파일 입력 오류" << endl;
        return 1; // 프로그램 종료
    }
    
    while (getline(fin, tmp)) {
        src.push_back(tmp);
    }
    fin.close();
    
    int address = 0; // 주소 초기화

    for (int i = 0; i < src.size(); i++) {//패스1
        vector<string> words;
        istringstream iss(src[i]);
        string word;
        
        while (iss >> word) {
            words.push_back(word); // 공백 단위로 분리
        }
        if(i==0){//첫 줄에서 시작주소 정하기
            address = stoi(words[2], nullptr, 16);
        }
        if (words.size() == 2) { // label 없음
            opcode = find_opcode(words[0], vec);
            operation newop(to_string(address), "-", words[0], words[1], opcode);// label 없음
            oper.push_back(newop);
        } else if (words.size() == 3) { // label 있음
            opcode = find_opcode(words[1], vec);
            operation newop(to_string(address), words[0], words[1], words[2], opcode); // label 있음
            oper.push_back(newop);
            if(words[1]=="byte" || words[1]=="word"||words[1]=="resb"||words[1]=="resw"){
                symtab.push_back(make_pair(words[0],address));//pair를 추가
                if(words[1]=="byte") address+= words[2].length()-6;//공간할당에 따른 다른 크기 고려
                else if(words[1]=="resb") address+= stoi(words[2])-3;
                else if(words[1]=="resw") address+= stoi(words[2])*3-3;
            }
            else{
                symtab.push_back(make_pair(words[0],address));
            }
        }
        if(i==0){//첫 줄(start)은 주소값이 안 변함
            address-=3;
        }
        // 주소 업데이트
        address += 3; // 예를 들어, 각 라인의 크기가 3으로 가정
    }
    for (int i = 0; i < src.size(); i++) {//패스2
        string operand_loc;
        if(oper[i].operate=="word"){
            oper[i].operand_loc =format("{:06X}",stoi(oper[i].operand));
        }
        else if(oper[i].operate=="byte"){
            oper[i].operand_loc = stringToHex(oper[i].operand);
            oper[i].operand_loc = oper[i].operand_loc.substr(4, oper[i].operand_loc.size()-6);
        }
        else if(oper[i].operate=="resb"||oper[i].operate=="resw"){
            oper[i].operand_loc = "";
        }
        else{//위치를 찾는다
            oper[i].operand_loc = format("{:04X}",find_operand(oper[i].operand, symtab));
        }
        if(oper[i].opcode =="**undefined operation code**"){
            oper[i].operror = true;//에러
        }
        if(find_operand(oper[i].operand, symtab) ==-1 && i !=0 && oper[i].operate != "word"&&oper[i].operate != "byte"&&oper[i].operate != "resb"&&oper[i].operate != "resw"&&oper[i].operate != "end"){
            oper[i].symerror=true;//에러
        }
    }
    // 결과 출력
    for (int i = 0; i < oper.size(); i++) {
        oper[i].result();
    }
    return 0;
}

