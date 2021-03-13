#include "Mips.h"

using namespace std;

unsigned int Mips::binToDec(string bin, int len)
{
    unsigned int dec = 0;

    for(int i=0; i<=len; i++)
            if(bin[i] == '1' )
                    dec = dec + pow(2, len-1-i) ;

    return dec;
}

Mips::Mips()
{
    cout<<"Mips iniciado"<<endl;
    tamInst = 0;

    for(int i=0; i<32; i++)
        Registers[i] = 0;
}

Mips::~Mips()
{
    delete [] memoria;
}

int Mips::controlALU()
{
    switch(uc.getALUOp())
    {
        case 0:
            return 2;
            break;
        case 1:
            return 6;
            break;
        case 2:
            // Desconsiderando o bit mais significativo
            bitset<5> funct;
            funct = IR;
            cout<<"campo funct: "<<funct<<endl;

            switch(funct.to_ulong())
            {
                case 0:
                    return 2;
                    break;
                case 2:
                    return 6;
                    break;
                case 4:
                    return 0;
                    break;
                case 5:
                    return 1;
                    break;
                case 12:
                    return 7;
                    break;
                default:
                    cout<<"Error funct"<<endl;
            }
            break;
    }
    cout<<"Error alucontrol"<<endl;
    return 0;
} 


void Mips::ALU()
{
    int a, b, aluresult;
    cout<<"ALU ok"<<endl;

    //Multiplexadores
    if(uc.getALUSrcA())
        a = A;
    else
        a = pc;

    cout<<"a:"<<A<<endl;

    switch(uc.getALUSrcB())
    {  
        case 0:
            b = B;
            break;
        case 1:
            b = 4/4;
            break;
        case 2:
            b = SignExtension();
            cout<<"b: "<<b<<endl;
            break; 
        case 3:
            b = SignExtension();
            break;
        default:
            cout<<"error aluscrb"<<endl;
    }

    //ALUop
    switch(controlALU())
    {
        case 2:
            aluresult = a+b;
            break;
        case 6:
            aluresult = a-b;
            if(aluresult == 0)
                zeroALU = true;
            else
                zeroALU = false;
            //MultiplexPc(ALUout);
            break;
        case 0:
            //and
            /* bitset<32> testA = a;
            bitset<32> testB = b;

            testA = testA & testB;
            aluresult = testA.to_ulong() */;
            break;
        case 1:
            //or
            /* bitset<32> testA = a;
            bitset<32> testB = b;

            testA = testA | testB;
            aluresult = testA.to_ulong(); */
            break;
        case 7:
            //slt
            aluresult = (a<b) ? 1:0;
            break;
        default:
            cout<<"error alu"<<endl;
    }


    ALUout = aluresult;

}

int Mips::SignExtension()
{
    bitset<16> res = IR;
    bitset<32> ext;
    cout<<res<<endl;

    int convertido;
    if(res[15] == 1)
    {
        res = (~res);//inverte res ATENÇÃO
        convertido = (res.to_ulong() + 1) * -1;
    }
    else
        convertido = res.to_ulong();
    /* ext = convertido;
    cout<<res<<endl;
    cout<<convertido<<endl;
    cout<<ext<<endl; */

    return convertido;
}


void Mips::MemoryData()
{
    int add;

    // Multiplexador
    if(uc.getIorD())
        add = ALUout;
    else
        add = pc;
    ////////////////

    if(uc.getMemWrite())
    {
        memoria[add] = B;
    }

    if(uc.getMemRead())
    {
        if(uc.getIRWrite())
            IR = memoria[add];
        MDR = memoria[add];
    }

}



void Mips::MultiplexPc(int result)
{

    if(uc.getPCWrite() || (uc.getPCWriteCond() &&  zeroALU))
    {
        switch(uc.getPCSource())
        {
            case 0:
                pc = result;
                break;
            case 1:
                pc = ALUout;
                break;
            case 2:
                pc = result;
                break;
        }
    }

}

void Mips::Reg()
{
    bitset<5> rs, rt, rd;

    rd = IR >> 11;
    rt = IR >> 16;
    rs = IR >> 21;

    cout<<"rt: "<<rt<<endl;

    if(uc.getRegwrite())
    {
        int store;

        if(uc.getMemtoReg())
            store = MDR;
        else
            store = ALUout;

        cout<<"store: "<<ALUout<<endl;

        if(uc.getRegDst())
            Registers[rd.to_ulong()] = store;
        else
            Registers[rt.to_ulong()] = store;
    }

    if(uc.getMemRead())
    {
        A = Registers[rs.to_ulong()];
        B = Registers[rt.to_ulong()];
    }
 

}

void Mips::Desvio()
{
    if(uc.getPCWrite())
    {
        bitset <32> pcSig = pc>>28;
        pcSig = pcSig<<28;

        bitset<26> j = IR;

        int result = pcSig.to_ulong() + j.to_ulong();

        MultiplexPc(result);
    }
}

void Mips::decodInstr(int instr)
{
    bitset<32> binInstr;
    bitset<26> address;
    bitset<16> immediate;
    bitset<6> opcode, funct;
    bitset<5> rs, rt, rd, shamt;

    //binInstr = instr;
    //cout<<binInstr<<endl;
    // 1000 0000 0001 1010 1010 1010 1110 1011

    address = instr;
    immediate = instr;
    funct = instr;
    shamt = instr >> 6;
    rd = instr >> 11;
    rt = instr >> 16;
    rs = instr >> 21;

    //opcode
    opcode = instr >> 26;

    // operações
    uc.setOpcode(opcode.to_ulong());
    Reg();

}

void Mips::leTxt(string nometxt)
{
    // Leitura do arquivo externo
    FILE *arq;
    char linha[32];
    char *result;
    int i;
    string bin;

    nometxt += ".txt";
    const char * c = nometxt.c_str();

    // Abre um arquivo TEXTO para LEITURA
    arq = fopen(c, "r");

    if (arq == NULL)  // Se houve erro na abertura
        {
            printf("Problemas na abertura do arquivo\n");
            exit(0);
        }

    i = 0;
    while (!feof(arq))
        {
            //cout<<"i: "<<i<<endl;
            result = fgets(linha, 64, arq);
            if (result)
                {
                    //cout<<"resultado: "<<tamInst<<endl;
                    bin = result;
                    memoria[tamInst] = binToDec(bin, 32);
                    tamInst++;
                }
            i++;
        }

        for(int i=0; i<tamInst; i++)
        {
            cout<<memoria[i]<<endl;
        }



    fclose(arq);
}

void Mips::start()
{
    string nometxt;
    cout<<"Digite o nome do arquivo: ";
    getline(cin, nometxt);
    leTxt(nometxt);

    etapa01();
    cout<<"pc: "<<pc<<endl;
    cout<<endl;
    etapa02();
    cout<<endl;
    etapa03();
    cout<<endl;
    etapa04();
    cout<<endl;
    //etapa05();


    for(int i=0; i<32; i++)
    {
        cout<<"register: "<<Registers[i]<<endl;
    }

    //while(pc<tamInst)
    {
        //ciclo01();
        //ciclo02();
        //ciclo03();
        //ciclo04();
        //ciclo05();  
    }



}


void Mips::etapa01()
{
    cout<<"ciclo 01"<<endl;
    MemoryData();

    ALU();
    
    // gambiarra
    MultiplexPc(ALUout);
}



void Mips::etapa02()
{
    cout<<"ciclo 02"<<endl;
    uc.setSinalEtapa2();
    decodInstr(IR);
    ALU();
}



void Mips::etapa03()
{
    if(uc.getState())
    {
        cout<<"ciclo 03"<<endl;
        uc.setSinalEtapa3();
        Desvio();
        ALU();
    }  
}



void Mips::etapa04()
{
    if(uc.getState())
    {
        cout<<"ciclo 04"<<endl;
        uc.setSinalEtapa4();

        if(uc.getRegwrite())
            Reg();
        
        if(uc.getMemWrite())
            MemoryData();
    }
}


void Mips::etapa05()
{
    cout<<"ciclo 05"<<endl;
    uc.setSinalEtapa5();
    Reg();
}