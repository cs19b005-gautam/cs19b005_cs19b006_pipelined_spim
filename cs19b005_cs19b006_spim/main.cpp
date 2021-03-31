//	1. label identification
//	2. label pointer array
//bubble sort MIPS assmenbly code
//blt	$t0,$t1,label

//1.instruction Fetch
//2.Instruction Decode/	Register Fetch
//3.Execute
//4.Memory
//5.Write Back

/*the total working of MIPS has been changed compared to last time as last time all stages were done like one but now as the
 *stages are necessary here every result from respective stages has to be taken in order for doing that we have had to add some
 *new functions for code and change some code in main had gone through changes and old functions were not removed because we
 *wanted to make this simulator supporting umpipelined, data_forwarding_pipelined and data_not_forwarding_pipelined due to lack
 *of time it was not done*/

#include <bits/stdc++.h>

using namespace std;
std::vector<string> vec;//file lines one by one
//std::set<string> commands = {,"add","sub","lw","j",,"mul","sw"};

int program_counter;	//	NEED TO BE INTIALISED IN MAIN MOST PROBABLY
//int 4X 10240 , char -> 40960 //40KB
int memory[10240] = { 0 }; //1 element->1 byte, 1KB->1024 bytes, 4KB->4096,this memory has 40KB/  //character 4 bits - 1 // 2) string
int memory_ptr = 0; //memory_ptr
int reg[32] = { 0 };
int low_high[2] = { 0 };
int data_enable = 1;
int forward_data;
struct int_data_label {//label:	.word	23,34,54
	string label;
	std::vector<int> values;
	int memory_index;
};
std::vector<int_data_label> int_data_label_array;
//int int_regs_reserved[5] = { 0 };	0->PC,1->EPC,2->BadVAddr,3->Status				/*Status usually have a value I guess*/
//float fp_register_single_precision[32] = { 0 };
//double fp_register_double_precision[32] = { 0 };
//int fp_regs_reserved[4] = { 0 };
//int enable_text_segment = 0;
std::vector<string> registers1 = { "$0","$1","$2","$3","$4","$5","$6","$7","$8","$9","$10","$11","$12","$13","$14","$15","$16","$17","$18","$19","$20","$21","$22","$23","$24","$25","$26","$27","$28","$29","$30","$31" };
std::vector<string> registers2 = { "$r0","$at","$v0","$v1","$a0","$a1","$a2","$a3","$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7","$t8","$t9","$k0","$k1","$gp","$sp","$s8","$ra" };

/*class stage_instruction_clock_info {

	bool no_work, IF, ID_RF, EX, MEM, WB, stall, branch_taken;
	string instruction, opcode, operand1, operand2, operand3;
	int offset, result, rd, rs, rt, cycle;

};*/

std::vector<string> stall_instructions;//for storing indexes of vec vector fo knowing list of instructions with stalls
std::vector<int> stall_PC;
struct stage_info {
	bool no_work = false;
	bool IF = false;
	bool ID_RF = false;
	bool EX = false;
	bool MEM = false;
	bool WB = false;
	bool stall = false;
	bool branch_taken = false;
	string instruction = "";
	string opcode = "";
	string operand1 = "";
	string operand2 = "";
	string operand3 = "";
	int PC;
	int offset = -1;
	int rd = -1;
	int rs = -1;
	int rt = -1;
	int result;
	int cycle = -1;
	int num_registers = 0;
};

std::vector<std::vector<stage_info>> pipeline_table;

void data_to_memory(int_data_label d) {
	d.memory_index = memory_ptr;
	int i = 0;
	for(;memory_ptr<d.values.size();memory_ptr++){
		memory[memory_ptr] = d.values[i];
		i++;
	}
	int_data_label_array.push_back(d);
}

struct text_label {
	string label;
	int line_index;
};

std::vector<text_label> text_label_array;

void jump(string str) {
	for (int i = 0; i < text_label_array.size(); i++) {
		if (text_label_array[i].label == str) {
			program_counter = (text_label_array[i].line_index);
			return;
		}
	}
}

bool int_str(string str) {
	bool only_int_char_exist = true;
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '-') {
			continue;
		}
		else if (str[i] < '0' || str[i]>'9') {
			only_int_char_exist = false;
		}
	}
	return only_int_char_exist;
}

int register_index(string str) {
	if (str == "$zero")
		return 0;
	int index = -1;
	for (int i = 0; i < 32; i++) {
		if (str == registers1[i] || str == registers2[i]) {
			index = i;;
			break;
		}
	}
	return index;
}

bool contains_register(string str) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '$') {
			return true;
		}
	}
	return false;
}

int get_data_memory_index(string str) {
	for (int i = 0; i<int_data_label_array.size(); i++) {
		if (int_data_label_array[i].label == str) {
			return int_data_label_array[i].memory_index;
		}
	}
}

void get_offset_and_register(string str, string* reg_str, string* offset) {
	int i;
	for (i = 0; i < str.length(); i++) {
		if (str[i] == '(')
			break;
	}
	*offset = str.substr(0, i);
	*reg_str = str.substr(i + 1, str.length() - 2 - i);
}

int load_store_mem(string str) {//this function is for getting adresses to registers||we can get adresses by 0($t0) , ($t0) , array, array($t0)
	if (!contains_register(str)) {//lablel
		return get_data_memory_index(str);
	}
	else {
		string offset = "";
		string register_string = "";
		if (str[0] == '(' && str[str.length() - 1] == ')') { //offset = ""  register_string = "$t0"		($t0)
			register_string = str.substr(1, str.length() - 1 - 1);
			int num = reg[register_index(register_string)];
			num = num / 4;
			return num;
		}
		else{//0($t0), array($t0)
			int i;
			for (i = 0; i < str.length(); i++) {
				if (str[i] == '(') {
					break;
				}
			}
			offset = str.substr(0, i);
			register_string = str.substr(i + 1,str.length() - i - 2);
			int num = reg[register_index(register_string)];
			num = num / 4;
			if (int_str(offset)) {//32($t0)
				int o = stoi(offset);
				o = o / 4;
				return (num + o);
			}
			else {
				int o = get_data_memory_index(offset);
				return (o + num);
			}
		}

	}
}



void perform_string_data_line_operation(string opcode,string operand1, string operand2){//making .data into memort
	int_data_label temp_int_data_label;
	temp_int_data_label.label = opcode.substr(0, opcode.length() - 1);
	if (int_str(operand2)) {
		temp_int_data_label.values.push_back(stoi(operand2));
	}
	else {
		int num = 0;
		for (int i = 0; i < operand2.length(); i++) {
			if (operand2[i] == ',') {
				temp_int_data_label.values.push_back(num);
				num = 0;
			}
			else if(operand2[i]=='0' && operand2[i+1]==',' && operand2[i-1]==',') {
				temp_int_data_label.values.push_back(0);
			}
			else {
				if (i == operand2.length() - 1) {
					num = (10*num) + (operand2[i]-'0');
					temp_int_data_label.values.push_back(num);
				}
				else {
					num = (10*num) + (operand2[i] - '0');
				}
			}
		}
	}
	//data_word.push_back(l);
}

void perform_string_line_operation(string opcode) {//	MAYBE BEST IF WE PASS A PAPRAMETER AS A STRING PARAMETER?
	if (opcode == "syscall") {
		//evaluate$v0;
		//if $v0<=4	print
		//else $v0>=5	take input into $v0
	}
}

void perform_string_line_operation(string opcode, string operand1) {//	MAYBE BEST IF WE RETURN A STRING POINTER F
	//if	opcode	register		=>perform arithematic operation
	//		mflo, mfhi
	//if	j	label				=>jump to pointer
	if (opcode == "mflo") {
		reg[register_index(operand1)] = low_high[0];
	}
	else if (opcode == "mfhi") {
		reg[register_index(operand1)] = low_high[1];
	}
	else if (opcode == "j") {
		jump(operand1);
	}
}

void perform_string_line_operation(string opcode, int num1) {
	//invalid
}

void perform_string_line_operation(string opcode, string operand1, string operand2) {//instruction decode
	//load,store,div,mul.move
	if (opcode == "div") {
		low_high[0] = reg[register_index(operand1)] / reg[register_index(operand2)];
		low_high[1] = reg[register_index(operand1)] % reg[register_index(operand2)];
	}
	else if (opcode == "mul") {

	}
	else if (opcode == "move") {
		reg[register_index(operand1)] = reg[register_index(operand2)];
	}
	else if (opcode == "lw") {
		reg[register_index(operand1)] = memory[load_store_mem(operand2)];
	}
	else if (opcode == "la") {
		reg[register_index(operand1)] = load_store_mem(operand2);
	}
	else if (opcode == "sw") {
		memory[load_store_mem(operand2)] = reg[register_index(operand1)];
	}
}

void perform_string_line_operation(string opcode, int num1, string operand2) {
	//invalid
}

void perform_string_line_operation(string opcode, string operand1, int num2) {
	if (opcode == "li") {
		reg[register_index(operand1)] = num2;
	}
}

void perform_string_line_operation(string opcode, int num1, int num2) {
	//invalid
}

void perform_string_line_operation(string opcode, string operand1, string operand2, string operand3) {
	if (opcode == "add") {
		int result;
		result = reg[register_index(operand2)] + reg[register_index(operand3)];//execute//num2+get_register_index(operand3);
		reg[register_index(operand1)] = result;
	}
	else if (opcode == "sub") {
		int result;
		result = reg[register_index(operand2)] - reg[register_index(operand3)];//execute//num2+get_register_index(operand3);
		reg[register_index(operand1)] = result;
	}
	else if (opcode == "beq") {
		if (reg[register_index(operand1)] == reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "bne") {
		if (reg[register_index(operand1)] != reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "blt") {
		if (reg[register_index(operand1)] < reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "bgt") {
		if (reg[register_index(operand1)] > reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "bge") {
		if (reg[register_index(operand1)] >= reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "ble") {
		if (reg[register_index(operand1)] <= reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "slt") {
		reg[register_index(operand1)] = reg[register_index(operand2)] < reg[register_index(operand3)];
	}
	else if (opcode == "sgt") {
		reg[register_index(operand1)] = reg[register_index(operand2)] > reg[register_index(operand3)];
	}
}

void perform_string_line_operation(string opcode, string operand1, string operand2, int num3) {
	if (opcode == "add") {
		reg[register_index(operand1)] = reg[register_index(operand2)] + num3;
	}
	else if (opcode == "sub") {
		reg[register_index(operand1)] = reg[register_index(operand2)] - num3;
	}
	else if (opcode == "mul") {

	}
}

void perform_string_line_operation(string opcode, string operand1, int num2, string operand3) {
	if (opcode == "add") {
		int result;
		result = num2 + reg[register_index(operand3)];//execute//num2+get_register_index(operand3);
		reg[register_index(operand1)] = result;//set_result(operand1)
	}
	else if (opcode == "sub") {
		int result;
		result = num2 - reg[register_index(operand3)];//execute//num2+get_register_index(operand3);
		reg[register_index(operand1)] = result;//set_result(operand1)
	}
	else if (opcode == "mul") {
		int result;
		result = num2 * reg[register_index(operand3)];//execute//num2+get_register_index(operand3);
		reg[register_index(operand1)] = result;//set_result(operand1)
	}
}

void perform_string_line_operation(string opcode, string operand1, int num2 , int num3) {
	if (opcode == "add") {
		int result;
		result = num2+num3;//execution
		reg[register_index(operand1)] = result ;//write back
	}
	else if (opcode == "sub") {
		int result;
		result = num2 - num3;//execution
		reg[register_index(operand1)] = result;//write back
	}
	else if (opcode == "mul") {
		int result;
		result = num2 * num3;//execution
		reg[register_index(operand1)] = result;//write back
	}
}

void perform_string_line_operation(string opcode, int num1, string operand2, string operand3) {
	//invalid
}

void perform_string_line_operation(string opcode, int num1, string operand2, int num3) {
	//invalid
}

void perform_string_line_operation(string opcode, int num1, int num2, string operand3) {
	//invalid
}

void perform_string_line_operation(string opcode, int num1, int num2, int num3) {
	//invalid
}


void file_line_text_read(string str,string* opc,string* op1,string*op2,string*op3,int* number1,int* number2, int* number3) {//opcode->li,add,sw,lw,sub,mul   operands(1/2/3)->destination registers,1/2source registers,   .dat
	//instruction fetch
	if (str == "" || str == "\t" || str == "\t\t" || str == "\t\t\t") {
		return;
	}

	//how to implement an array without different data-types in C++  ?

	//.text		enable Instruction read
	string opcode = "";//can be a opcode or a label defintion
	string operand1 = "";// can be a register or a number also  or a label callee
	string operand2 = "";
	string operand3 = "";
	//strings without instructions

	int i, num_of_operands = 0;
	int start_opcode_index = -1;
	for (i = 0; i < str.length(); i++) {	//make into 3 or 4 parts		"	li $v0"
		if (str[i] == ' ' || str[i] == '\t') {
			if (start_opcode_index == -1) {
				continue;
			}
			else {
				break;
			}
		}
		else if (str[i] >= 'a' && str[i] <= 'z' && start_opcode_index == -1) {
			start_opcode_index = i;
		}
	}
	opcode = str.substr(start_opcode_index, i - start_opcode_index);
	*opc = opcode;

	//operand1
	int start_operand_index = -1;
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == ',') {
			if (start_operand_index != -1) {
				break;
			}
			else {
				continue;
			}
		}
		else if (str[i] == '$') {
			start_operand_index = i;
			break;
		}//find out where next space is
		else if (str[i] <= '9' && str[i] >= '0') {
			start_operand_index = i;
			break;
		}
		else if ((opcode == "j" || opcode == "jal") && ((str[i] <= 'z' && str[i] >= 'a') || (str[i] <= 'Z' && str[i] >= 'A'))) {	// for label
			start_operand_index = i;
			break;
		}
	}
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == ',') {
			break;
		}
	}
	if (start_operand_index != -1) {
		operand1 = str.substr(start_operand_index, i - start_operand_index);
		*op1 = operand1;
		num_of_operands++;
	}
	int num1;
	if (operand1 != "" && int_str(operand1)) {
		num1 = stoi(operand1);
		*number1 = num1;
	}


	//operand2
	start_operand_index = -1;
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == ',') {
			if (start_operand_index != -1) {
				break;
			}
			else {
				continue;
			}
		}
		else {
			start_operand_index = i;
			break;
		}//find out where next space is

	}
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == ',') {
			break;
		}
	}
	if (start_operand_index != -1) {
		operand2 = str.substr(start_operand_index, i - start_operand_index);
		*op2 = operand2;
		num_of_operands++;
	}
	int num2;
	if (operand2 != "" && int_str(operand2)) {
		num2 = stoi(operand2);
		*number2 = num2;
	}



	//operand3
	start_operand_index = -1;
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == ',') {
			if (start_operand_index != -1) {
				break;
			}
			else {
				continue;
			}
		}
		else {
			start_operand_index = i;
			break;
		}//find out where next space is

	}
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == ',') {
			break;
		}
	}
	if (start_operand_index != -1) {
		operand3 = str.substr(start_operand_index, i - start_operand_index);
		*op3 = operand3;
		num_of_operands++;
	}
	int num3;
	if (operand3 != "" && int_str(operand3)) {
		num3 = stoi(operand3);
		*number3 = num3;
	}

	if (i != str.length()) {
		operand3 = str.substr(start_operand_index, i - start_operand_index);
		num_of_operands++;
	}

	//cout << *opc << " " << *op1 << " " << *op2 << endl;
	if (opcode == "syscall" && reg[register_index("$v0")] == 10) {
		program_counter = vec.size();
		return;
	}

}

void file_line_data_read(string str) {
	if (str == "" || str == "\t" || str == "\t\t" || str == "\t\t\t") {
		return ;
	}
	string label = "", type = "", values = "";
	int start_index = -1;
	int i = 0;
	for (i = 0; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t') {
			if (start_index == -1) {
				continue;
			}
			else {
				break;
			}
		}
		else {
			if (start_index == -1) {
				start_index = i;
			}
		}
	}
	if (i != 0 && str.substr(start_index, i - start_index) == ".data") {
		return ;
	}
	else if (i != 0) {
		label = str.substr(start_index, i - start_index - 1);
	}
	start_index = -1;
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t') {
			if (start_index == -1) {
				continue;
			}
			else {
				break;
			}
		}
		else {
			if (start_index == -1) {
				start_index = i;
			}
		}
	}
	if (start_index >= 0) {
		type = str.substr(start_index, i - start_index);
	}
	start_index = -1;
	for (; i < str.length(); i++) {
		if (str[i] == ' ' || str[i] == '\t') {
			if (start_index == -1) {
				continue;
			}
			else {
				break;
			}
		}
		else {
			if (start_index == -1) {
				start_index = i;
			}
		}
	}
	if (start_index >= 0) {
		values = str.substr(start_index, str.length() - start_index);
	}
	int val;
	int num = 0;
	std::vector<int> arr;
	for (int i = 0; i < values.length(); i++) {
		if (values[i] == ',') {
			arr.push_back(num);
			num = 0;
		}
		else if (values[i] <= '9' && values[i] >= '0') {
			num = 10 * num + (values[i] - '0');
			if (i == values.length() - 1) {
				arr.push_back(num);
			}
		}
	}
	int_data_label temp;
	temp.label = label;
	temp.values = arr;
	temp.memory_index = -1;
	data_to_memory(temp);
}


void file_line_read(string str,string* instruction) {
	if (str == "" || str == "\t" || str == "\t\t" || str == "\t\t\t") {
		return;
	}
	string s;
	int i,index = -1;
	for (i = 0; i < str.length(); i++) {
		if (str[i] == ' '||str[i]=='\t') {
			if (index==-1) {
				continue;
			}
			else {
				break;
			}
		}
		else {
			if (index == -1) {
				index = i;
			}
		}
	}
	s = str.substr(index, i - index);


	if (s == ".data") {
		data_enable = 1;
	}
	else if (s == ".text") {
		data_enable = 0;

	}

	if (data_enable) {
		file_line_data_read(str);
	}
	else {
		if(s[s.length()-1]!=':' && s[0]!='.')
			*instruction = str;
		//file_line_text_read(str);
	}
}


void execute(string opcode, int rd, int rs, int rt, int offset, string operand2, string operand3, int* result) {
	int op2, op3;

	if (opcode == "add") {
		if (int_str(operand3)) {
			*result = reg[rs] + stoi(operand3);
		}
		else {
			*result = reg[rs] + reg[rt];
		}
	}
	else if (opcode == "sub") {
		if (int_str(operand3)) {
			*result = reg[rs] - stoi(operand3);
		}
		else {
			*result = reg[rs] - reg[rt];
		}
	}
	else if (opcode == "slt") {
		*result = (reg[rs] < reg[rt]);
	}
	else if (opcode == "lw") {
		*result = offset + (reg[rs]/4);
	}
	else if (opcode == "sw") {
		*result = offset + (reg[rs]/4);
	}
	else if (opcode == "li") {
		*result = stoi(operand2);
	}
}

void memory_stage(string opcode,int rd, string operand1, string operand2,int offset, int* result) {
	if (opcode == "lw") {
		*result = memory[*result];
	}
	else if (opcode == "sw") {
		memory[*result] = reg[rd];
	}
}

void reg_fetch(string opcode, string operand1, string operand2, string operand3, int* num1, int* num2, int* num3, int* offset,int* result) {
	if (opcode == "j") {
		jump(operand1);
	}
	else if (opcode == "beq") {
		*num1 = register_index(operand1);
		*num2 = register_index(operand2);
		if (reg[register_index(operand1)] == reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "bne") {
		*num1 = register_index(operand1);
		*num2 = register_index(operand2);
		if (reg[register_index(operand1)] != reg[register_index(operand2)]) {
			jump(operand3);
		}
	}
	else if (opcode == "add") {
		*num1 = register_index(operand1);
		*num2 = register_index(operand2);
		*num3 = register_index(operand3);//if it's a number num2,rt would be 32
	}
	else if (opcode == "sub") {
		*num1 = register_index(operand1);
		*num2 = register_index(operand2);
		*num3 = register_index(operand3);//if it's a number num2,rt would be 32
	}
	else if (opcode == "lw") {
		*num1 = register_index(operand1);//dest
		//cout << *num1 << endl;
		string offset_str, reg_str;
		get_offset_and_register(operand2, &reg_str, &offset_str );
		//cout << reg_str << " " << offset_str << endl;
		*num2 = register_index(reg_str);
		if (int_str(offset_str)) {
			*offset = stoi(offset_str)/4;
		}
		else {
			*offset = load_store_mem(offset_str);
			//get address of label offset
		}
	}
	else if (opcode == "la") {
		*num1 = register_index(operand1);//dest
		*num2 = load_store_mem(operand2);
		*result = *num2;
		//get address of label
	}
	else if (opcode == "sw") {
		*num1 = register_index(operand1);//source
		string offset_str, reg_str;
		get_offset_and_register(operand2, &reg_str, &offset_str);
		*num2 = register_index(reg_str);
		if (int_str(offset_str)) {
			*offset = stoi(offset_str)/4;
		}
		else {
			*offset = load_store_mem(offset_str);
			//get address of label offset
		}
	}
	else if (opcode == "slt") {
		*num1 = register_index(operand1);
		*num2 = register_index(operand2);
		*num3 = register_index(operand3);
	}
	else if (opcode == "li") {
		*num1 = register_index(operand1);
	}
}

void write_back(string opcode, int rd, int result) {
	if (opcode == "add" || opcode == "sub" || opcode == "slt" || opcode == "li" || opcode == "lw" || opcode == "la") {
		reg[rd] = result;
	}
}

void inc_all_vec_in_pipeline_table(int a) {
	for (int i = a; i < pipeline_table.size(); i++) {
		pipeline_table[i].resize(1000);
	}
}

int main() {
	cout << "This simulator has used lot of memory and contains bigger loops please wait for few seconds" << endl << endl;
	pipeline_table.resize(1000);
	for (int i = 0; i < pipeline_table.size(); i++) {
		pipeline_table[i].resize(1000);
	}


	string fileName;
	cout << "Please provide your file path" << endl << "(Please mention the file extension along with the file name while providing the file path) : " << endl ;


	getline(cin, fileName);
	fstream file;
	file.open(fileName, ios::in | ios::out );
	if (!file.is_open())
		cout << "ERROR OCCURRED WHILE OPENING THE FILE." << endl << "PLEASE PROVIDE A VALID FILE PATH.";
	else {
		cout << endl << "FILE OPENED SUCCESSFULLY" << endl ;
		string line;

		cout << endl << "Do want to have data forwarding in your pipelining ( 1 for YES, 0 for NO ) : ";
		cin >> forward_data;
		cout << endl;
		cout << "############################################################################################" << endl;
		cout << "\t\t\t FILE CODE" << endl;
		cout << "############################################################################################" << endl;
		while (file.good()) {
			getline(file, line);
			cout << line << endl;
			vec.push_back(line);///Add label vector which contains string pointer
			if (line.length()!=0 && line[line.length() - 1] == ':') {//Disadv : if any label in .text those have to be in a line separately
				//cout << &(vec[vec.size() - 1]) << endl;
				text_label temp_label;
				temp_label.label = line.substr(0, line.length() - 1);
				temp_label.line_index = vec.size()-1;
				text_label_array.push_back(temp_label);
				//pushing label into labels vector so whenever label is seen 'j label' program counter jumps to label
			}
		}
		program_counter = 0;
		int num_stalls = 0;
		int n;
		int end_cycle = 0;
		int number_of_performed_instructs = 0;
		int start_clock = 0;//this helps to us in knowing the first stage of each instruction
		for (; program_counter<vec.size();) {
			int stage_number = 0;//denotes what stage it is when passed to function

			int ptr = 0;//for each stage progressing in case of stalls
			for (int i = 0; i < start_clock; i++) {
				pipeline_table[number_of_performed_instructs][i].no_work = true;
			}//keeping all before numbers as dummy no work


			if (number_of_performed_instructs > pipeline_table.size() - 100) { //in case the number of instructions incrfease
				int len = pipeline_table.size();
				pipeline_table.resize(100);
				for (int j = len; j < pipeline_table.size(); j++) {
					pipeline_table[j].resize(1000);
				}
			}

			if (start_clock > pipeline_table[number_of_performed_instructs].size() - 20) {//if in case of unavailabel clocks
				pipeline_table[number_of_performed_instructs].resize(20);
			}

			stage_info temp;
			string instruction = "";
			string opcode = "";
			string operand1 = "";
			string operand2 = "";
			string operand3 = "";
			int num1 = 0;
			int num2 = 0;
			int num3 = 0;
			int offset = -1;
			n = program_counter;
			temp.instruction = instruction;
			temp.PC = program_counter;
			file_line_read(vec[program_counter],&instruction);//take indices too
			program_counter++;//instruction fetch
			temp.no_work = true;
			if (instruction == "") {
				continue;
			}
			if (number_of_performed_instructs == 0) {
				temp.IF = true;
				pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
				ptr++;
			}
			else {//no need for data forwarding in IF
				if (pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].IF == true && pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].stall == false) {//it won't be on same cycle because evry time start_clock gets incremented and IF need only 1 cycle
					temp.IF = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
				}
				else if(pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].IF == true && pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].stall == true){
					for (int j = start_clock + ptr; pipeline_table[number_of_performed_instructs-1][start_clock + ptr].stall == true || pipeline_table[number_of_performed_instructs-1][start_clock + ptr].ID_RF != true; j++, ptr++){
						temp.stall = true;
                        pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
                        ptr++;
                        temp.stall = false;

					}//stalls for different instructions occurring at same cycle will be considered only one stall not multiple stalls
					temp.IF = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					stall_instructions.push_back(instruction);
					stall_PC.push_back(number_of_performed_instructs+1);
				}
			}
			stage_number++;
			//pipelining(stage_number,temp,number_of_performed_instructs-1);


			//send instruction_fetch results to another function for pipelining this instruction
			file_line_text_read(instruction, &opcode, &operand1, &operand2, &operand3, &num1, &num2, &num3);//instruction decode
			temp.opcode = opcode;
			temp.operand1 = operand1;
			temp.operand2 = operand2;
			temp.operand3 = operand3;

			int rd = -1;
			int rs = -1;
			int rt = -1;
			int result = -1;
			reg_fetch(opcode, operand1, operand2, operand3, &rd, &rs, &rt, &offset, &result);
			stage_number++;
			temp.rd = rd;
			temp.rs = rs;
			temp.rt = rt;
			temp.offset = offset;
			temp.result = result;
			if (number_of_performed_instructs>0 && (pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].ID_RF == true && pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].stall == true)) {//in case of stalls occured whil prefoeming ID_RF to prev instruction at same cycle
				for(int j = start_clock + ptr; j>=0; j++) {
					if (pipeline_table[number_of_performed_instructs - 1][start_clock + ptr].EX != true) {
						break;
					}
					temp.stall = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					temp.stall = false;
					ptr++;
				}
                stall_PC.push_back(number_of_performed_instructs+1);
				stall_instructions.push_back(instruction);//for same clock cycle it won't two stalls evven if we get stalls at 2 instruct at same clock cycle
			}

			if (opcode == "beq" || opcode == "bne" || opcode == "j") {//change all rd to rs and rs to rt and make rd = -1 aas no dest_reg will be used
				temp.rd = -1;
				temp.rs = rd;
				temp.rt = rs;// later for the next instructions  the first operand is not actually dest reg
				// n and pc will be different if branch taken
				if (n != program_counter) {
					temp.branch_taken = true;
				}
				//pipelining(stage_number,temp,number_of_performed_instructs - 1);


				if (opcode == "j") {//jump statement does not depend on any statement
					temp.ID_RF = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					temp.EX = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					temp.MEM = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					temp.WB = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					number_of_performed_instructs++;
					start_clock++;
					continue;
				}
				if (forward_data == 0) {
					temp.ID_RF = true;
					int temp_reg1,temp_reg2;//te,p-reg1 is for 1st lasr instruct and temp_reg2 is for 2nd last reg

					for (int j = start_clock + ptr; j>=0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 1][j].rd >= 0) {
							temp_reg1 = pipeline_table[number_of_performed_instructs - 1][j].rd;
							break;
						}
					}
					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 2][j].rd >= 0) {
							temp_reg2 = pipeline_table[number_of_performed_instructs - 2][j].rd;
							break;
						}
					}
					if (rd == temp_reg1 || rs == temp_reg1) {
						//always a stall after 1st ID/RF
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
						temp.stall = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr]= temp;
						num_stalls++;
						temp.stall = false;
						stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
						ptr++;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else if (rd == temp_reg2 || rs == temp_reg2) {
						//always a 2nd ID_RF after 1st one
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else {
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
				}
				else if (forward_data == 1) {// there can be any unavoidable stalls even in case of data_forwarding - have to do this
					temp.ID_RF = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					int temp_reg1, temp_reg2;
					string temp_opcode1, temp_opcode2;
					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 1][j].rd >= 0) {
							temp_reg1 = pipeline_table[number_of_performed_instructs - 1][j].rd;
							temp_opcode1 = pipeline_table[number_of_performed_instructs - 1][j].opcode;
							break;
						}
					}
					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 2][j].rd >= 0) {
							temp_reg2 = pipeline_table[number_of_performed_instructs - 2][j].rd;
							temp_opcode2 = pipeline_table[number_of_performed_instructs - 2][j].opcode;
							break;
						}
					}

					if(((temp_opcode1 == "lw" || temp_opcode1 == "la") && (rd == temp_reg1 || rs == temp_reg1)) && ((temp_opcode2 == "lw" || temp_opcode2 == "la") && (rd == temp_reg2 || rs == temp_reg2))){

						temp.stall = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						temp.stall = false;
						num_stalls++;
						stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else if ((temp_opcode1 == "lw" || temp_opcode1 == "la") && (rd == temp_reg1 || rs == temp_reg1)) {

						temp.stall = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						temp.stall = false;
						num_stalls++;
						stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else if ((temp_opcode2 == "lw" || temp_opcode2 == "la") && (rd == temp_reg2 || rs == temp_reg2)) {
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
				}
				//make all other stages for beq and bne here only as they are dummy
				temp.EX = true;
				pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
				ptr++;
				temp.MEM = true;
				pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
				ptr++;
				temp.WB = true;
				pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
				ptr++;
				if (temp.branch_taken == true) {
					start_clock+=2; // eventhough the next instruction is fetched aimultaneously that would be useless and continue is used  so +=2
				}
				number_of_performed_instructs++;
				continue;// as all stagews in pipelining are made going to next instruction
			}
			else if (opcode == "sw") {
				//we won't use sw, beq, bne for 1st time
				if (forward_data == 0) {
					temp.ID_RF = true;
					int temp_reg1, temp_reg2;//te,p-reg1 is for 1st lasr instruct and temp_reg2 is for 2nd last reg

					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 1][j].rd >= 0) {
							temp_reg1 = pipeline_table[number_of_performed_instructs - 1][j].rd;
							break;
						}
					}
					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 2][j].rd >= 0) {
							temp_reg2 = pipeline_table[number_of_performed_instructs - 2][j].rd;
							break;
						}
					}
					if (rd == temp_reg1 || rs == temp_reg1) {
						//always a stall after 1st ID/RF
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
						temp.stall = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						num_stalls++;
						temp.stall = false;
						stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
						ptr++;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else if (rd == temp_reg2 || rs == temp_reg2) {
						//always a 2nd ID_RF after 1st one
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else {
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
				}
				else if (forward_data == 1) {// there can be any unavoidable stalls even in case of data_forwarding - have to do this
					temp.ID_RF = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
					int temp_reg1, temp_reg2;
					string temp_opcode1, temp_opcode2;
					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 1][j].rd >= 0) {
							temp_reg1 = pipeline_table[number_of_performed_instructs - 1][j].rd;
							temp_opcode1 = pipeline_table[number_of_performed_instructs - 1][j].opcode;
							break;
						}
					}
					for (int j = start_clock + ptr; j >= 0; j--) {
						//going back until rd is received
						if (pipeline_table[number_of_performed_instructs - 2][j].rd >= 0) {
							temp_reg2 = pipeline_table[number_of_performed_instructs - 2][j].rd;
							temp_opcode2 = pipeline_table[number_of_performed_instructs - 2][j].opcode;
							break;
						}
					}
					if(((temp_opcode1 == "lw" || temp_opcode1 == "la") && (rd == temp_reg1 || rs == temp_reg1)) && ((temp_opcode2 == "lw" || temp_opcode2 == "la") && (rd == temp_reg2 || rs == temp_reg2))){

						temp.stall = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						temp.stall = false;
						num_stalls++;
						stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else if ((temp_opcode1 == "lw" || temp_opcode1 == "la") && (rd == temp_reg1 || rs == temp_reg1)) {

						temp.stall = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						temp.stall = false;
						num_stalls++;
						stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
					else if ((temp_opcode2 == "lw" || temp_opcode2 == "la") && (rd == temp_reg2 || rs == temp_reg2)) {
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
					}
				}
			}
			else {//all others apart from j,beq,bne,sw will have first entered as destination register for WB
				if (number_of_performed_instructs == 0) {
					temp.ID_RF = true;
					pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
					ptr++;
				}
				else {//incomplete until next break point
					if (forward_data == 1) {
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
						int temp_reg1, temp_reg2;
						string temp_opcode1, temp_opcode2;
						for (int j = start_clock + ptr; j >= 0; j--) {
							if (pipeline_table[number_of_performed_instructs - 1][j].rd >= 0) {
								temp_opcode1 = pipeline_table[number_of_performed_instructs - 1][j].opcode;
								temp_reg1 = pipeline_table[number_of_performed_instructs - 1][j].rd;
								break;
							}
						}
						for (int j = start_clock + ptr; j >= 0 && number_of_performed_instructs - 2 >=0; j--) {
							if (pipeline_table[number_of_performed_instructs - 2][j].rd >= 0) {
								temp_opcode2 = pipeline_table[number_of_performed_instructs - 2][j].opcode;
								temp_reg2 = pipeline_table[number_of_performed_instructs - 2][j].rd;
								break;
							}
						}

                        if(((temp_opcode1 == "lw" || temp_opcode1 == "la") && (rt == temp_reg1 || rs == temp_reg1)) && ((temp_opcode2 == "lw" || temp_opcode2 == "la") && (rt == temp_reg2 || rs == temp_reg2))){
                            temp.stall = true;
                            pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
                            temp.stall = false;
                            num_stalls++;
                            stall_instructions.push_back(instruction);
                        stall_PC.push_back(number_of_performed_instructs+1);
                            pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
                            ptr++;
                        }
                        else if ((temp_opcode1 == "lw" || temp_opcode1 == "la") && (rt == temp_reg1 || rs == temp_reg1)) {

							temp.stall = true;
							pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
							temp.stall = false;
							num_stalls++;
							stall_instructions.push_back(instruction);
                            stall_PC.push_back(number_of_performed_instructs+1);
							pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
							ptr++;
						}
						else if ((temp_opcode2 == "lw" || temp_opcode2 == "la") && (rt == temp_reg2 || rs == temp_reg2)) {
							pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
							ptr++;
						}
					}
					else if (forward_data == 0) {
						temp.ID_RF = true;
						pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
						ptr++;
						int temp_reg1, temp_reg2;
						string temp_opcode1, temp_opcode2;
						for (int j = start_clock + ptr; j >= 0; j--) {
							if (pipeline_table[number_of_performed_instructs - 1][j].rd >= 0) {
								temp_opcode1 = pipeline_table[number_of_performed_instructs - 1][j].opcode;
								temp_reg1 = pipeline_table[number_of_performed_instructs - 1][j].rd;
								break;
							}
						}
						for (int j = start_clock + ptr; j >= 0 && number_of_performed_instructs - 2>=0; j--) {
							if (pipeline_table[number_of_performed_instructs - 2][j].rd >= 0) {
								temp_opcode2 = pipeline_table[number_of_performed_instructs - 2][j].opcode;
								temp_reg2 = pipeline_table[number_of_performed_instructs - 2][j].rd;
								break;
							}
						}
						if (temp_opcode1 == "beq" || temp_opcode1 == "bne" || temp_opcode1 == "j" || temp_opcode1 == "sw") {
							//may have possible hazards(dependencies) in
							if (temp_opcode2 == "beq" || temp_opcode2 == "bne" || temp_opcode2 == "j" || temp_opcode2 == "sw") {
								//there won't be any stalls so no need to keep any statements
							}
							else {
								//there's a chance of presence of stalls because of last 2nd prev instruct
								if (temp_reg2 == rs || temp_reg2 == rt) {
									for (int j = start_clock + ptr; pipeline_table[number_of_performed_instructs - 2][j].WB != true; j++) {
										temp.stall = true;
										pipeline_table[number_of_performed_instructs][j] = temp;
										temp.stall = false;
										num_stalls++;
										ptr++;
									}
                                    stall_instructions.push_back(instruction);
                                    stall_PC.push_back(number_of_performed_instructs+1);
									pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
									ptr++;
								}
							}
						}
						else {
							if (temp_opcode2 == "beq" || temp_opcode2 == "bne" || temp_opcode2 == "j" || temp_opcode2 == "sw") {
								//chances are there can be stalls with respect to last 1st instruct
								if (temp_reg1 == rs || temp_reg1 == rt) {//even if there are 2 dependencies(hazards) RF happens at 2nd instruct  WB only
									for (int j = start_clock + ptr; pipeline_table[number_of_performed_instructs - 1][j].WB != true; j++) {
										temp.stall = true;
										pipeline_table[number_of_performed_instructs][j] = temp;
										temp.stall = false;
										num_stalls++;
										ptr++;
									}
									stall_instructions.push_back(instruction);
                                    stall_PC.push_back(number_of_performed_instructs+1);
									pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
									ptr++;
								}
							}
							else{ // Having hazardal dependencies at both 1st prev and 2nd prev instructs
                                if (temp_reg1 == rs || temp_reg1 == rt) {//even if there are 2 dependencies(hazards) RF happens at 2nd instruct  WB only
									for (int j = start_clock + ptr; pipeline_table[number_of_performed_instructs - 1][j].WB != true; j++) {
										temp.stall = true;
										pipeline_table[number_of_performed_instructs][j] = temp;
										temp.stall = false;
										num_stalls++;
										ptr++;
									}
									stall_instructions.push_back(instruction);
                                    stall_PC.push_back(number_of_performed_instructs+1);
									pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
									ptr++;
								}
							}

						}

					}
				}

			}
			//pipelining for ID_RF with and without data forwarding

			/*else {
				//pipelining(stage_number, temp, number_of_performed_instructs - 1);
			}*/	//register fetch
			//send instruction decode and register fetch results to sam function and figure out about dependencies and data forw

			execute(opcode, rd, rs, rt, offset,operand2,operand3, &result);
			stage_number++;
			temp.result = result;
			//there can be stalls move away until the prev instruction completes your EX completes and starts MEM for prev instruct
			temp.EX = true;//anyhow EX of registers doesn't play factor in dependency but however there will be data forwarded to EX but since in this simulator we can already fetch the values we can directly assign and any stall occurs due t0 ID_RF so anyhow ID_RF for current instruction does the part we don't have to check for prev instructions for this case in simulator

			pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
			ptr++;
			//pipelining(stage_number, temp, number_of_performed_instructs - 1);
			//send execute results to the same function


			memory_stage(opcode, rd, operand1, operand2, offset, &result);
			stage_number++;
			//there can be stalls move away until the prev instruction completes your MEM completes and starts EX for prev instruct
			temp.MEM = true;
			pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
			ptr++;
			//pipelining(stage_number, temp, number_of_performed_instructs - 1);
			//send memory functions to dame function


			write_back(opcode,rd,result);
			temp.WB = true;
			pipeline_table[number_of_performed_instructs][start_clock + ptr] = temp;
			ptr++;
			stage_number++;

			//pipelining(stage_number, temp, number_of_performed_instructs - 1);
			//send write back functions to dame function

			number_of_performed_instructs++;
			start_clock++;
			end_cycle = start_clock + ptr;
		}
		end_cycle++;//the total number of cycles taken for given MIPS assembly code
		program_counter = n;
		cout << "#####################################################################################" << endl;
		cout << "\t\t\t\tPIPELINE INFORMATION" << endl;
		cout << "#####################################################################################" << endl;
		cout << "Number of stalls = " << num_stalls << endl << endl;
		float floa_instructs = (float)(number_of_performed_instructs);
		float total_cycles = (float)(end_cycle);
		cout << "IPC (Instructions Per Cycle)  =  " << floa_instructs/total_cycles << endl << endl;
        cout << "LIST OF STALL INSTRUCTIONS :" << endl<<"(Because of the loops you may see the same instruction repeated but you can see that the" << endl << " instruction is not shown twice by seeing but which nth instruction performed can bee seen)"<< endl;
        cout << endl << "nth stall instruction\t" << "instruction" << endl;
        cout << "while executing " << endl;
        cout << "----------------------\t" << "--------------------------" << endl;
        for(int p = 0;p < stall_instructions.size(); p++ ){
            cout << stall_PC[p] << "\t\t" <<stall_instructions[p] << endl;
        }
		cout << "*************************************************************************************" << endl;

		cout << "#####################################################################################" << endl;
		cout << "\t\t\t\tREGISTERS" << endl;
		cout << "#####################################################################################" << endl;

		for (int i = 0; i < 16; i++) {
			cout << "R" << i;
			cout << " ";
			if (i <= 9) {
				cout << " ";
			}
			cout << "[" << registers2[i] << "] = ";
			cout << reg[i] << "\t\t\t";
			cout << "R" << i+16;
			cout << " ";
			cout << "[" << registers2[i+16] << "] = ";
			cout << reg[i+16] << endl;
		}

		cout << "#####################################################################################" << endl;
		cout << "\t\t\t\tMEMORY" << endl;
		cout << "#####################################################################################" << endl;
		cout << endl << "Addresses  -> Values" << endl;
		cout << "=========     ======" << endl;
		for (int it = 0; it < memory_ptr; it++) {
			cout << &(memory[it]) << "   ->   " << memory[it] << endl;
		}

		file.close();
	}

	cout << endl << endl << "**************************************************************************************" << endl;
	return 0;
}
/*struct
*	int result,rd,rs,rt,PC;
* bool if,id_rf,ex,mem,wb,stall;
*	[i-1][j]						taken						string opcode,op1,op2,op3;
*   [i][j]
*/
