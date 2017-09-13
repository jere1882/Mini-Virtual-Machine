#include <stdio.h>
#include <ctype.h>

#include <stdlib.h>
#include <string.h>
#include "machine.h"

//DEFINICIONES DE VARIABLES EXTERNAS
extern FILE *yyin;   // http://aquamentus.com/flex_bison.html 
struct Instruction code[512];
int count; //Numero de instrucciones en el código
struct Machine machine;
const char *regname[REGS] = { "\%zero", "\%pc", "\%sp", "\%r0","\%r1","\%r2","\%r3", "\%flags"};


// DADO UN STRING, SI ES EL NOMBRE DE UN REGISTRO DEVUELVE EL INDICE.
int reg(const char* r) {	
  r++; // Skip %
  if (!strcmp(r,"zero")) return 0;
  if (!strcmp(r,"pc")) return 1;
  if (!strcmp(r,"sp")) return 2;
  if (!strcmp(r,"r0")) return 3;
  if (!strcmp(r,"r1")) return 4;
  if (!strcmp(r,"r2")) return 5;
  if (!strcmp(r,"r3")) return 6;
  if (!strcmp(r,"flags")) return 7;
  printf("Unkown Register %s\n",r);
  abort();
}

// IMPRIME EL VALOR DE TODOS LOS REGISTROS DE LA MAQUINA 
void dumpMachine() {	
  int i;
  printf("**************** Machine state at PC=%d **************\n", machine.reg[PC]);
  for (i=0;i<REGS;i++) 
    if (strlen(regname[i])==0) continue; 
    else printf("%s\t\t= \t\t%d \t\t%x\n",regname[i],machine.reg[i],machine.reg[i]);
  printf("******************************************************\n");
}

// COPIA 4 BYTES
void copy_4bytes (void *a, void *b){
		int *to = (int *) a;
		int *from = (int *) b;
		*to = *from;
}


//FUNCIÓN AUXILIAR PARA SETEAR LAS BANDERAS
setflags(int zero,int equal, int lower){      
	machine.reg[FLAGS] =  (lower << LOWER_BIT_FLAGS ) | (equal << EQUAL_BIT_FLAGS) | (zero << ZERO_BIT_FLAGS);
}

//NOTIFICA UN ERROR Y LA LINEA EN QUE SE PRODUJO.
raise (char* message){
	int line = machine.reg[PC];
	printf ("(linea %d) ERROR: %s\n",line,message);
	abort();
}


//EJECUTA UNA INSTRUCCIÓN
void runIns(struct Instruction i) {	  
  switch (i.op) {
	
	int temp,temp2;
		
    case NOP:
		break;
		
	// INSTRUCCIONES DE CARGA Y MOVIMIENTO (MOV,LW,SW)
	
	case MOV: // dst <- src			 
		if (i.src.type==REG){
			temp=machine.reg[i.src.val];
			if (i.dst.type==REG) {
				machine.reg[i.dst.val] = temp;
				break;
			} 
			else if (i.dst.type==MEM){
				copy_4bytes(&machine.memory[i.dst.val], &temp);
				break;
			}
		}
		else if (i.dst.type==REG) {
				if (i.src.type==IMM)
					machine.reg[i.dst.val]=i.src.val;
				else
					copy_4bytes(&machine.reg[i.dst.val], &machine.memory[i.src.val]);
				break;
		}
		else if (i.src.type == IMM & i.dst.type == MEM){
			copy_4bytes (&machine.memory[i.dst.val], &i.src.val);
			break;
		}
		raise("MOV espera que alguno de sus argumentos sea un registro.");
		break;

	case SW: //Save Word: Guarda el valor apuntado por src en dst
		if (i.src.type!=REG)
			raise ("SW espera que su primer argumento sea un registro");
		
		switch (i.dst.type) {
			case IMM:
				raise("SW espera que su destino sea un registro o dire memoria.");
			case REG:
				temp = machine.reg[i.dst.val];
				break;
			case MEM:
				temp = i.dst.val;
				break;
		}
		copy_4bytes(&machine.memory[temp],&machine.reg[i.src.val]);
		break;
		
	
	case LW: //Load Word: Guarda el valor apuntado por src en el registro dst
		switch (i.src.type){
			case IMM:
				raise ("LW espera que su fuente no sea un valor inmediato");
			case REG:
				temp = machine.reg[i.src.val]; 
				break;
			case MEM:
				temp = i.src.val;
				break;
		}
		if (i.dst.type!= REG)
			raise ("LW espera que su destino un registro");
		
		copy_4bytes(&machine.reg[i.dst.val],&machine.memory[temp]);
		break;


	//INSTRUCCIONES ARITMETICAS (Inm -> Reg o Reg -> Reg)
		
	case ADD: 
		if (i.dst.type!=REG)
			raise ("ADD espera que su destino sea un registro");

		switch (i.src.type){
			case IMM:
				machine.reg[i.dst.val]+= i.src.val;
				break;
			case REG:
				machine.reg[i.dst.val]+=machine.reg[i.src.val];
				break;
			case MEM:
				raise ("ADD espera que su fuente no este en memoria");
		}
		break;
		
		
	case INC: 
		if (i.src.type!=REG)
			raise ("ADD espera que su argumento sea un registro");
		machine.reg[i.src.val]++;
		break;	

	case DEC: 
		if (i.src.type!=REG)
			raise ("DEC espera que su argumento sea un registro");
		machine.reg[i.src.val]--;
		break;
				
	case SUB: // dst <- dst - src
		if (i.dst.type!=REG)
			raise ("SUB espera que su destino sea un registro");
			
		switch (i.src.type){
			case IMM:
				machine.reg[i.dst.val]-= i.src.val;
				break;
			case REG:
				machine.reg[i.dst.val]-=machine.reg[i.src.val];
				break;
			case MEM:
				raise ("SUB espera que su fuente no este en memoria");
		}
		break;
	
	
	case MUL: // dst <- dst * src
		if (i.dst.type!=REG)
			raise ("MUL espera que su destino sea un registro");
		switch (i.src.type){
			case IMM:
				machine.reg[i.dst.val]*= i.src.val;
				break;
			case REG:
				machine.reg[i.dst.val]*=machine.reg[i.src.val];
				break;
			case MEM:
				raise ("MUL espera que su fuente no este en memoria");
		}
	break;

	case DIV: // dst <- dst / src
		if (i.dst.type!=REG)
			raise ("DIV espera que su destino sea un registro");
		switch (i.src.type){
			case IMM:
				temp = i.src.val;
				if (temp==0)
					raise ("Division por cero");
				machine.reg[i.dst.val] /= temp;
				break;
			case REG:
				temp = machine.reg[i.src.val];
				if (temp==0)
					raise ("Division por cero");
				machine.reg[i.dst.val] /= temp;
				break;
			case MEM:
				raise ("DIV espera que su fuente no este en memoria");
		}
	break;
	
	case CMP: // hace la resta virtual dst - src y setea las flags.
		if (i.dst.type!=REG)
			raise ("CMP espera que su destino sea un registro");
		switch (i.src.type){
			case IMM:
				temp = machine.reg[i.dst.val] - i.src.val;
				break;
			case REG:
				temp = machine.reg[i.dst.val]- machine.reg[i.src.val];
				break;
			case MEM:
				raise ("CMP espera que su destino no este en memoria");
		}
		
		if (temp<0)
		  setflags(0,0,1); 
        if (temp==0)
		  setflags(1,1,0);
        if (temp>0)
		  setflags(0,0,0);
        
        break;
		
		
	//INSTRUCCIONES LOGICAS (BIT A BIT)

	case XOR: // dst <- dst xor src
		if (i.dst.type!=REG)
			raise ("XOR espera que su destino sea un registro");
		switch (i.src.type){
			case IMM:
				machine.reg[i.dst.val]^= i.src.val;
				break;
			case REG:
				machine.reg[i.dst.val]^=machine.reg[i.src.val];
				break;
			case MEM:
				raise ("XOR  espera que su fuente no este en memoria");
		}
		break;
	
	case OR: // dst <- dst | src
		if (i.dst.type!=REG)
			raise ("OR espera que su destino sea un registro");
		switch (i.src.type){
			case IMM:
				machine.reg[i.dst.val]|= i.src.val;
				break;
			case REG:
				machine.reg[i.dst.val]|=machine.reg[i.src.val];
				break;
			case MEM:
				raise ("OR espera que su fuente no este en memoria");
		}
		break;
	
	case AND: // dst <- dst & src
		if (i.dst.type!=REG)
			raise ("AND  espera que su destino sea un registro");
		switch (i.src.type){
			case IMM:
				machine.reg[i.dst.val]&= i.src.val;
				break;
			case REG:
				machine.reg[i.dst.val]&=machine.reg[i.src.val];
				break;
			case MEM:
				raise ("AND espera que su fuente no este en memoria");
		}
		break;
	
	case NOT : // dst <- ~dst
		if (i.src.type!=REG)
			raise ("NOT espera que su argumento sea un registro");

		machine.reg[i.src.val] = ~ machine.reg[i.src.val];
		break;
	
	
	//INSTRUCCIONES DE MANEJO DE PILA
	
	case PUSH: // guarda src en el tope de la pila. 
			   // src debe ser un registro o un valor inmediato
		switch (i.src.type){
			case IMM:
				machine.reg[SP]-=4;
				copy_4bytes(&machine.memory[machine.reg[SP]],&i.src.val);
				break;
			case REG:
				machine.reg[SP]-=4;
				copy_4bytes(&machine.memory[machine.reg[SP]],&machine.reg[i.src.val]);
				break;
			case MEM:
				raise ("PUSH espera que src no este en memoria");
		}
		break;
		

	case POP: // escribe en el registro dst el valor que está en el tope de la pila
		if (i.src.type!=REG)
			raise ("POP espera que dst sea un registro");
		else if (machine.reg[SP] == MEM_SIZE)
			raise ("POP. La pila esta vacia.");

		copy_4bytes(&machine.reg[i.src.val],&machine.memory[machine.reg[SP]]);
		machine.reg[SP]+=4;
		break;
		
	// ENTRADA - SALIDA 
			
	case PRINT:
		switch(i.src.type){
				case IMM:
					printf ("%d \n", i.src.val);
					break;
				case REG:
					printf("%d \n", machine.reg[i.src.val]);
					break;
				case MEM:
					copy_4bytes (&temp, &machine.memory[i.src.val]);
					printf("%d \n", temp); 
					break;
		}
		break;

	
	case READ: // Lee un entero y lo guarda en el registro dst
		if (i.src.type != REG)
			raise ("READ espera que dst sea un registro");
		scanf ("%d",&temp);
		machine.reg[i.src.val]=temp;
		break;
	
	// SALTOS

    case JMP: // src será un IMM, pues processLabels habrá modificado ya la etiqueta
		if (i.src.type!=IMM)
				raise ("JMP espera que su argumento sea una etiqueta o valor inmediato");
		machine.reg[PC]=i.src.val;
		break;
	case CALL: // src es un IMM
		if (i.src.type!=IMM)
			raise ("CALL espera que su argumento sea una etiqueta o valor inmediato");
		machine.reg[SP]-=4;
		copy_4bytes(&machine.memory[machine.reg[SP]],&machine.reg[PC]); //Pusheo la dirección de retorno
		machine.reg[PC]=i.src.val;
		break;
	case RET: 
		copy_4bytes(&machine.reg[PC],&machine.memory[machine.reg[SP]]);
		machine.reg[SP]+=4;		
		break;		
    case JMPE: // src será un IMM, pues processLabels habrá modificado ya la etiqueta
		if (i.src.type!=IMM)
				raise ("JMPE espera que su argumento sea una etiqueta o valor inmediato");
		machine.reg[PC]= ISSET_EQUAL ? i.src.val : machine.reg[PC]+1;
		break;


    case JMPL: // src será un IMM, pues processLabels habrá modificado ya la etiqueta
		if (i.src.type!=IMM)
				raise ("JMPL espera que su argumento sea una etiqueta o valor inmediato");
		machine.reg[PC]= ISSET_LOWER ? i.src.val : machine.reg[PC]+1;
		break;
		
    case LOOP: // src será un IMM,  lo decrementa r3 y salta si r3!=0 
		if (i.src.type!=IMM)
				raise ("JMPL espera que su argumento sea una etiqueta o valor inmediato");
		machine.reg[R3]--;
		machine.reg[PC] = machine.reg[R3] ?  i.src.val : machine.reg[PC] +1  ; 
		break; 
    
    // OPERACIONES DE BITS. 
	
	case SHR: // mueve los bits del registro dst, src veces a la derecha. src es un registro o imm.
		switch (i.src.type){
			case MEM:
				raise (" SHR espera que src no este en memoria ");	
			case REG:
				temp = machine.reg[i.src.val];
				break;
			case IMM:
				temp = i.src.val;
				break;
		}
		// En temp esta guardado el corrimiento
		if (temp<0)
			raise (" SHR espera que src sea no negativo ");
		if (i.dst.type!=REG)
			raise (" SHR espera que dst sea un registro ");
		machine.reg[i.dst.val] =  ((unsigned) machine.reg[i.dst.val] ) >> temp ; // esto me la complico. es necesario porque el shift de c es el 
																				 // aritm o logico segun el tpo de datos. no sabia.
		break;
		
	case SHL: // mueve los bits del registro dst, src veces a la iz. src es un registro o imm.
		switch (i.src.type){
			case MEM:
				raise (" SHL espera que src no este en memoria ");	
			case REG:
				temp = machine.reg[i.src.val];
				break;
			case IMM:
				temp = i.src.val;
				break;
		}
		// En temp esta guardado el corrimiento
		if (temp<0)
			raise (" SHL espera que src sea no negativo ");
		if (i.dst.type!=REG)
			raise (" SHL espera que dst sea un registro ");
		machine.reg[i.dst.val] <<= temp ;
		break;		
		
		
	case SAR: // mueve los bits del registro dst, src veces a la derecha. src es un registro o imm.
		switch (i.src.type){
			case MEM:
				raise (" SAR espera que src no este en memoria ");	
			case REG:
				temp = machine.reg[i.src.val];
				break;
			case IMM:
				temp = i.src.val;
				break;
		}
		// En temp esta guardado el corrimiento
		if (temp<0)
			raise (" SAR espera que src sea no negativo ");
		if (i.dst.type!=REG)
			raise (" SAR espera que dst sea un registro ");
		machine.reg[i.dst.val] >>= temp ;
		break;
		
	case SAL: // mueve los bits del registro dst, src veces a la derecha. src es un registro o imm.
		switch (i.src.type){
			case MEM:
				raise (" SAL espera que src no este en memoria ");	
			case REG:
				temp = machine.reg[i.src.val];
				break;
			case IMM:
				temp = i.src.val;
				break;
		}
		// En temp esta guardado el corrimiento
		if (temp<0)
			raise (" SALespera que src sea no negativo ");
		if (i.dst.type!=REG)
			raise (" SAL espera que dst sea un registro ");
		machine.reg[i.dst.val] <<= temp ;
		break;
		
		
	case ROL: // rota los bits del registro dst, src veces a la iz. src es un registro o imm.
		switch (i.src.type){
			case MEM:
				raise (" ROL espera que src no este en memoria ");	
			case REG:
				temp = machine.reg[i.src.val];
				break;
			case IMM:
				temp = i.src.val;
				break;
		}
		// En temp esta guardado el corrimiento
		if (temp<0)
			raise (" ROL espera que src sea no negativo ");
		if (i.dst.type!=REG)
			raise (" ROL espera que dst sea un registro ");
			
		temp = temp % 32; // NO HAGO VUELTAS ENTERAS
		temp2 = machine.reg[i.dst.val]; // VALOR SOBRE EL Q TRABAJAMOS
		machine.reg[i.dst.val] =  ( ((unsigned)( temp2 & (-1 << (32 - temp) ) )) >> (32 - temp) ) | (temp2 << temp);
		break;	
	
	
	case ROR: // rota los bits del registro dst, src veces a la iz. src es un registro o imm.
		switch (i.src.type){
			case MEM:
				raise (" ROR espera que src no este en memoria ");	
			case REG:
				temp = machine.reg[i.src.val];
				break;
			case IMM:
				temp = i.src.val;
				break;
		}
		// En temp esta guardado el corrimiento
		if (temp<0)
			raise (" ROR espera que src sea no negativo ");
		if (i.dst.type!=REG)
			raise (" ROR espera que dst sea un registro ");
			
		temp = temp % 32; // NO HAGO VUELTAS ENTERAS
		temp2 = machine.reg[i.dst.val]; // VALOR SOBRE EL Q TRABAJAMOS
		machine.reg[i.dst.val] = ( (temp2  & ( ~(-1 << temp)) ) << (32-temp) ) | (((unsigned)temp2) >> temp) ;
		break;	


	

    default:
      printf("Instruction %d not implemented\n",i.op);
      abort();
    }
    
}

//EJECUTA UN PROGRAMA LINEA A LINEA
void run(struct Instruction *code) {	
  machine.reg[PC] = 0; // Start at first instruction
  machine.reg[SP] = MEM_SIZE; 

  Opcode actual_op;
  
  while (code[machine.reg[PC]].op!=HLT) {
	actual_op = code[machine.reg[PC]].op;
    runIns(code[machine.reg[PC]]);
	
    if (actual_op!=JMP && actual_op!=JMPE && actual_op!=JMPL && actual_op!= CALL && actual_op != LOOP)
		machine.reg[PC]++;

  }
}

//REEMPLAZA LAS ETIQUETAS POR VALORES INMEDIATOS (Direcc. de memoria)
void processLabels() {
  int i,j,k;
  for (i=0;i<count;i++) {
    if (code[i].op==LABEL) {
      for (j=0;j<count;j++) {
        if (code[j].op == JMP || code[j].op== JMPE || code[j].op==JMPL || code[j].op == CALL || code[j].op== LOOP) {
          if (code[j].src.lab && strcmp(code[j].src.lab,code[i].src.lab)==0) {
            code[j].src.type=IMM;
            code[j].src.val=i;
            code[j].src.lab=NULL;
          }
        }
      }
      for (j=i;j<count-1;j++) {
        code[j]=code[j+1];
      }
      count--;
      
    }
  }
  for (j=0;j<count;j++) {
    if (code[j].op == JMP || code[j].op== JMPE || code[j].op==JMPL || code[j].op == CALL || code[j].op== LOOP) {
      if (code[j].src.lab) {
        printf("Jump to unkown label %s\n",code[j].src.lab);
        exit(-1);
      }
    }
  }
  
}


//SHOW FUNCTIONS
void printOperand(struct Operand s) {
  switch (s.type) {
    case IMM:
      printf("$%d", s.val);
      break;
    case REG:
      printf("%s", regname[s.val]);
      break;
    case MEM:
      printf("%d", s.val);
      break;
  } 

}
void printInstr(struct Instruction i) {
  switch (i.op) {
    case NOP:
      printf("NOP\n");
      break;
    case HLT:
      printf("HLT\n");
      break;
    case RET:
      printf("RET\n");
      break;      
    case MOV:
      printf("MOV ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case ADD:
      printf("ADD ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case MUL:
      printf("MUL ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case SUB:
      printf("SUB ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case DIV:
      printf("DIV ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case ROL:
      printf("ROL ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
     case ROR:
      printf("ROR ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
     case SHL:
      printf("SHL ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
     case SHR:
      printf("SHR ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
     case SAR:
      printf("SAR ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
     case SAL:
      printf("SAL ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case LW:
      printf("LW ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break; 
    case SW:
      printf("SW ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case JMPL:
      printf("JMPL ");
      printOperand(i.src);
      if (i.src.lab)
        printf("%s",i.src.lab);
      printf("\n");
      break;
    case CALL:
      printf("CALL ");
      printOperand(i.src);
      if (i.src.lab)
        printf("%s",i.src.lab);
      printf("\n");
      break;
    case LOOP:
      printf("LOOP ");
      printOperand(i.src);
      if (i.src.lab)
        printf("%s",i.src.lab);
      printf("\n");
      break;
    case JMPE:
      printf("JMPE ");
      printOperand(i.src);
      if (i.src.lab)
        printf("%s",i.src.lab);
      printf("\n");
      break;
    case JMP:
      printf("JMP ");
      printOperand(i.src);
      if (i.src.lab)
        printf("%s",i.src.lab);
      printf("\n");
      break;
    case LABEL:
      printf("LABEL %s",i.src.lab);
      printf("\n");
      break;
    case CMP:
      printf("CMP ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case AND:
      printf("AND ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case XOR:
      printf("XOR ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
    case OR:
      printf("OR ");
      printOperand(i.src);
      printf(",");
      printOperand(i.dst);
      printf("\n");
      break;
     
    case PRINT:
      printf("PRINT ");
      printOperand(i.src);
      printf("\n");
      break;
    case READ:
      printf("READ ");
      printOperand(i.src);
      printf("\n");
      break; 
    case POP:
      printf("POP ");
      printOperand(i.src);
      printf("\n");
      break; 
    case PUSH:
      printf("PUSH ");
      printOperand(i.src);
      printf("\n");
      break; 
    case DEC:
      printf("DEC ");
      printOperand(i.src);
      printf("\n");
      break; 
    case INC:
      printf("INC ");
      printOperand(i.src);
      printf("\n");
      break; 
    case NOT:
      printf("NOT ");
      printOperand(i.src);
      printf("\n");
      break; 
      
    default:
      printf("Instrucction not printed");
      printf("\n");
  }
}


int main(int argc, char *argv[]) {
  int i;
  FILE *fp = fopen(argv[1], "r");
  yyin = fp;
  yyparse();
  fclose(fp);
  processLabels();
  printf("Running the following code\n");
  for (i=0;i<count; i++)  {
    printf("%d: ",i);
    printInstr(code[i]);
  }
  printf("***************\n");
  /*
  int e1=1, e2=2, e3=3, e4=4, e5=5;
  copy_4bytes (&machine.memory[123],&e1);
  copy_4bytes (&machine.memory[127],&e2);
  copy_4bytes (&machine.memory[131],&e3);
  copy_4bytes (&machine.memory[135],&e4);
  copy_4bytes (&machine.memory[139],&e5);*/
  run(code);
  dumpMachine();
  

  return 0;

}
