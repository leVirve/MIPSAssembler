#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <vector>

using namespace std;

typedef unsigned int UINT32;
struct str{ char s[100]; str() { memset(s, 0, sizeof(s)); } };

map<string, int> regMap;
map<string, UINT32> opCodeMap;
map<string, UINT32> functMap;
map<string, UINT32> labelMap;
vector<str> instr;

FILE *I_input, *D_input;
FILE *I_output, *D_output;

UINT32 PC;
UINT32 instr_cnt;

void initialMapping();
void iimagesetup();
void dimagesetup();
void parseInstr();

int main()
{
    initialMapping();
    try {
        iimagesetup();
        parseInstr();
        // dimagesetup();
    } catch(const char* e) {
        printf("%s\n", e);
    }
    getchar();
    return 0;
}

void parseInstr()
{
    for(int i = 0; i < instr_cnt; ++i) {
        printf("--> %s\n", instr[i].s);

        UINT32 _instruction = 0;
        int k = 0;
        char targ[] = " ,$()\n", *tmp;
        char op[10] = {}, args[3][10];
        tmp = strtok(instr[i].s, targ);
        strcpy(op, tmp);

        while(tmp != NULL) {
            tmp = strtok(NULL, targ);
            if(tmp != NULL)
                strcpy(args[k++], tmp);
        }
        _instruction |= opCodeMap[op] << 26;
        switch(opCodeMap[op]) {
            case 0x0:
                switch(functMap[op]) {
                    case 0x00: case 0x03:
                        _instruction |= (atoi(args[1]) & 0x1f) << 21;
                        _instruction |= (atoi(args[0]) & 0x1f) << 16;
                        _instruction |= (atoi(args[2]) & 0x1f) << 11;
                        _instruction |= functMap[op];
                        printf("[R] %s rt: %d rd: %d c: %d \n", op, atoi(args[1]), atoi(args[0]), atoi(args[2]));
                        break;
                    case 0x08:
                        _instruction |= (atoi(args[0]) & 0x1f) << 21;
                        _instruction |= functMap[op];
                        printf("[R] %s rs: %d \n", op, atoi(args[0]));
                        break;
                    default:
                        _instruction |= (atoi(args[1]) & 0x1f) << 21;
                        _instruction |= (atoi(args[2]) & 0x1f) << 16;
                        _instruction |= (atoi(args[0]) & 0x1f) << 11;
                        _instruction |= functMap[op];
                        printf("[R] %s rs: %d rt: %d rd: %d\n", op, atoi(args[1]), atoi(args[2]), atoi(args[0]));
                        break;
                }
            break;
            case 0x23: case 0x21: case 0x25: case 0x20:
            case 0x24: case 0x2b: case 0x29: case 0x28:
                _instruction |= (atoi(args[2]) & 0x1f) << 21;
                _instruction |= (atoi(args[0]) & 0x1f) << 16;
                _instruction |= (atoi(args[1]) & 0xffff);
                _instruction |= functMap[op];
                printf("[I] %s rs: %d rt: %d c: %d\n", op, atoi(args[2]), atoi(args[0]), atoi(args[1]));
                break;
            case 0x08: case 0x0c: case 0x0d: case 0x0e: case 0x0a:
                _instruction |= (atoi(args[1]) & 0x1f) << 21;
                _instruction |= (atoi(args[0]) & 0x1f) << 16;
                _instruction |= (atoi(args[2]) & 0xffff);
                _instruction |= functMap[op];
                printf("[I] %s rs: %d rt: %d c: %d\n", op, atoi(args[1]), atoi(args[0]), atoi(args[2]));
                break;
            case 0x0f:
                _instruction |= (atoi(args[0]) & 0x1f) << 16;
                _instruction |= (atoi(args[1]) & 0xffff);
                _instruction |= functMap[op];
                printf("[I] %s rt: %d c: %d\n", op, atoi(args[0]), atoi(args[1]));
                break;
            case 0x04: case 0x05:
                _instruction |= (atoi(args[0]) & 0x1f) << 21;
                _instruction |= (atoi(args[1]) & 0x1f) << 16;
                _instruction |= (labelMap[args[2]] - (i + 1)) & 0xffff;
                printf("[I] %s rs: %X rt: %X c: %s (%X -> %X)\n", op, atoi(args[0]), atoi(args[1]), args[2],
                    labelMap[args[2]], (labelMap[args[2]] - (i + 1)) & 0xffff);
                break;
            case 0x02: case 0x03:
                _instruction |= (labelMap[args[0]] * 4 + PC) << 4 >> 6;
                printf("[J] %s c: %X\n", op, (labelMap[args[0]] * 4 + PC) << 4 >> 6);
                break;
            case 0x3f:
                _instruction |= 0xffffffff;
                printf("[Halt]\n");
        }
        printf("=> %08X\n", _instruction);
        fwrite(&_instruction, sizeof(_instruction), 1, I_output);
    }
    fclose(I_output);
}

void iimagesetup()
{
    I_input = fopen("I_image.txt", "r");
    I_output = fopen("iimage.txt", "wb");

    if(I_input == NULL) throw "I_image text file not found";
    
    char buf[512];

    instr_cnt = 0;
    fscanf(I_input, "%X%*c\n", &PC);
    printf("Initial PC = 0x%08X\n", PC);
    while(fgets(buf, sizeof(buf), I_input) != NULL) {
        str label_name, _instr;
        strcpy(label_name.s, strtok(buf, " "));
        strcpy(_instr.s, strtok(NULL, "\n"));
        
        labelMap[label_name.s] = instr_cnt++;
        instr.push_back(_instr);
    }
    printf("Total Instructions : %d\n", instr_cnt);

    fwrite(&PC, sizeof(PC), 1, I_output);
    fwrite(&instr_cnt, sizeof(instr_cnt), 1, I_output);
    fclose(I_input);
}

void dimagesetup()
{
    FILE* D_input = fopen("D_image.txt", "r");
    FILE* D_output = fopen("dimage.txt", "wb");

    if(D_input == NULL) throw "D_image text file not found";
    fclose(D_input);
}

void initialMapping()
{
    /**
     * Register name mapping
     */
    char name[3] = {'0','0','\0'};
    string s;
    regMap.clear();
    for(int i = 0; i < 8; ++i) {
        name[0] = 't'; name[1] = '0' + i;
        s.assign(name);
        regMap[s] = 8 + i;
    }
    for(int i = 0; i < 8; ++i) {
        name[0] = 's'; name[1] = '0' + i;
        s.assign(name);
        regMap[s] = 16 + i;
    }
    regMap["t8"] = 24; regMap["t9"] = 25;

    /**
     * Instruction name mapping
     */
    // R-Type 'funct code' mapping
    functMap["add"]  = 0x20;
    functMap["sub"]  = 0x22;
    functMap["and"]  = 0x24;
    functMap["or"]   = 0x25;
    functMap["xor"]  = 0x26;
    functMap["nor"]  = 0x27;
    functMap["nand"] = 0x28;
    functMap["slt"]  = 0x2A;
    functMap["sll"]  = 0x00;
    functMap["srl"]  = 0x02;
    functMap["sra"]  = 0x03;
    functMap["jr"]   = 0x08;
    
    opCodeMap["add"]  = 0x00;
    opCodeMap["sub"]  = 0x00;
    opCodeMap["and"]  = 0x00;
    opCodeMap["or"]   = 0x00;
    opCodeMap["xor"]  = 0x00;
    opCodeMap["nor"]  = 0x00;
    opCodeMap["nand"] = 0x00;
    opCodeMap["slt"]  = 0x00;
    opCodeMap["sll"]  = 0x00;
    opCodeMap["srl"]  = 0x00;
    opCodeMap["sra"]  = 0x00;
    opCodeMap["jr"]   = 0x00;

    // I-Type opcode mapping
    opCodeMap["addi"] = 0x08;
    opCodeMap["lw"]   = 0x23;
    opCodeMap["lh"]   = 0x21;
    opCodeMap["lhu"]  = 0x25;
    opCodeMap["lb"]   = 0x20;
    opCodeMap["lbu"]  = 0x24;
    opCodeMap["sw"]   = 0x2B;
    opCodeMap["sh"]   = 0x29;
    opCodeMap["sb"]   = 0x28;
    opCodeMap["lui"]  = 0x0F;
    opCodeMap["andi"] = 0x0C;
    opCodeMap["ori"]  = 0x0D;
    opCodeMap["nori"] = 0x0E;
    opCodeMap["slti"] = 0x0A;
    opCodeMap["beq"]  = 0x04;
    opCodeMap["bne"]  = 0x05;

    // J-Type opcode mapping
    opCodeMap["j"]    = 0x02;
    opCodeMap["jal"]  = 0x03;

    // special mapping
    opCodeMap["halt"] = 0x3F;
}