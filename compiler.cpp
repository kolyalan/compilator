#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cstdio>
#include <set>
using namespace std;

enum TYPE{UNDEF, RESERVED, ID, CONSTINT, CONSTREAL, CONSTSTR, CONSTCHAR, CONSTLOGIC, OPERATION, PUNCTUATION, BRACKETS};
enum VARTYPE{NONE, CHAR, UCHAR, SHORT, USHORT, INT, UINT, LL, ULL, FLOAT, DOUBLE, LDOUBLE, STR, BOOL, SUM, SUB, MULT, DIV, MOD, NOT, AND, OR, POW, MORE, LESS, MORE_EQ, LESS_EQ, EQUAL, NOT_EQ, INC, DEC, ASSIGN, ADDRESS, LINK, MOV, FALSE_MOV, SEMICOLON, UN_SUM, UN_SUB, CIN, COUT};
//			   0   1      2      3       4     5     6    7   8    9       10       11      12  13    14   15   16    17   18   19   20  21   22   23    24     25       26       27     28     29   30    31     32перем. 33переход 34    35      36         37      38
struct token {
	TYPE type = UNDEF;
	string value;
	int line;
	int position;
};

token c;

struct var {
	string name;
	VARTYPE type;
	void * value;
	var() {};
	var(string name, VARTYPE type) : name(name), type(type) {
		if (type == BOOL) value = new bool(0);
		else if (type == CHAR) value = new char(0);
		else if (type == UCHAR) value = new unsigned char(0);
		else if (type == SHORT) {
			value = new short int(0);
		} else if (type == USHORT) {
			value = new unsigned short int(0);
		} else if (type == INT) {
			value = new int(0);
		} else if (type == UINT) {
			value = new unsigned int(0);
		} else if (type == LL) {
			value = new long long int(0);
		} else if (type == ULL) {
			value = new unsigned long long int(0);
		} else if (type == FLOAT) {
			value = new float(0);
		} else if (type == DOUBLE) {
			value = new double(0);
		} else if (type == LDOUBLE) {
			value = new long double(0);
		} else if (type == STR) {
			value = new string("");
		}
	};
	void set_value(string val) {
		if (type == BOOL) {
			if (val == "true") *(bool*)value = true;
			else if (val == "false") *(bool*)value = false;
			else {
				cout << "Неопознанная, но почему-то логическая константа. Возможно, ошибка в компиляторе";
				throw c;
			}
		} else if (type == CHAR) *(char*)value = val[0];
		else if (type == UCHAR) *(unsigned char*)value = val[0];
		else if (type == SHORT) {
			sscanf(val.c_str(), "%hd", (short int *) value);
		} else if (type == USHORT) {
			sscanf(val.c_str(), "%hu", (unsigned short int *) value);
		} else if (type == INT) {
			sscanf(val.c_str(), "%d", (int *) value);
		} else if (type == UINT) {
			sscanf(val.c_str(), "%u", (unsigned int *) value);
		} else if (type == LL) {
			sscanf(val.c_str(), "%lld", (long long int *) value);
		} else if (type == ULL) {
			sscanf(val.c_str(), "%llu", (unsigned long long int *) value);
		} else if (type == FLOAT) {
			sscanf(val.c_str(), "%f", (float *) value);
		} else if (type == DOUBLE) {
			sscanf(val.c_str(), "%lf", (double *) value);
		} else if (type == LDOUBLE) {
			sscanf(val.c_str(), "%Lf", (long double *) value);
		} else if (type == STR) {
			*(string *)value = val;
		}
	}
};

struct hashtable {
	vector<var*> list[10000];
	long long hash(string s) {
		long long ans = 0;
		for (size_t i = 0; i < s.size(); i++) {
			ans = (ans*47 + s[i] - 'a'+1)%10000;
		}
		return ans;
	}
	bool insert(var * a) {
		int ind = hash(a->name);
		for (size_t i = 0; i < list[ind].size(); i++) {
			if (list[ind][i] != nullptr && list[ind][i]->name == a->name) return 0;
		}
		list[ind].push_back(a);
		return 1;
	}
	var * get(string a) {
		int ind = hash(a);
		for (size_t i = 0; i < list[ind].size(); i++) {
			if (list[ind][i] != nullptr && list[ind][i]->name == a) return list[ind][i];
		}
		return nullptr;
	}
};

vector<hashtable *> tids;
vector<VARTYPE> st;

void push(VARTYPE t) {
	st.push_back(t);
}

bool is_int_type(VARTYPE t) {
	return (t == CHAR || t == UCHAR || t == SHORT || t == USHORT || t == INT || t == UINT || t == LL || t == ULL);
}

bool is_num_type(VARTYPE t) {
	return (t == CHAR || t == UCHAR || t == SHORT || t == USHORT || t == INT || t == UINT || t == LL || t == ULL || t == FLOAT || t == DOUBLE || t == LDOUBLE);
}

void check_un() {
	//cout << "check_un()" << endl;
	VARTYPE t, op, ret;
	t = st.back();
	st.pop_back();
	op = st.back();
	st.pop_back();
	if (op == NOT) {
		if (t == BOOL) {
			ret = BOOL;
		} else {
			cout << "Несоответствие типов" << endl;
			throw c;
		}
	} else if (op == SUM || op == SUB) {
		if (is_num_type(t)) {
			ret = t;
		} else { 
			cout << "Несоответствие типов" << endl;
			throw c;
		}
	}
	push(ret);
}

void check_op() {
	VARTYPE t1, t2, op, ret;
	t2 = st.back();
	st.pop_back();
	op = st.back();
	st.pop_back();
	t1 = st.back();
	st.pop_back();
	if (op == SUM || op == SUB || op == MULT || op == DIV) {
		if (!is_num_type(t1) || !is_num_type(t2)) {
			cout << "Несоответствие типов операндов" << endl;
			throw c;
		}
		if (t1 == LDOUBLE || t2 == LDOUBLE) {
			ret = LDOUBLE;
		} else if (t1 == DOUBLE || t2 == DOUBLE) {
			ret = DOUBLE;
		} else if (t1 == FLOAT || t2 == FLOAT) {
			ret = FLOAT;
		} else if (t1 == ULL || t2 == ULL) {
			ret = ULL;
		} else if (t1 == LL || t2 == LL) {
			ret = LL;
		} else if (t1 == UINT || t2 == UINT) {
			ret = UINT;
		} else if (t1 == INT || t2 == INT) {
			ret = INT;
		} else if (t1 == USHORT || t2 == USHORT) {
			ret = USHORT;
		} else if (t1 == SHORT || t2 == SHORT) {
			ret = SHORT;
		} else if (t1 == UCHAR || t2 == UCHAR) {
			ret = UCHAR;
		} else if (t1 == CHAR || t2 == CHAR) {
			ret = CHAR;
		} else {
			cout << "Несоответствие типов" << endl;
			throw c;
		}
	} else if (op == POW) {
		if (!is_num_type(t1) || !is_num_type(t1)) {
			cout << "Несоответствие типов" << endl;
			throw c;
		}
		ret = DOUBLE;
	} else if (op == AND || op == OR) {
		if  (t1 == BOOL && t2 == BOOL) {
			ret = BOOL;
		} else {
			cout << "Несоответствие типов" << endl;
			throw c;
		}
	} else if (op == MORE || op == MORE_EQ || op == LESS || op == LESS_EQ || op == EQUAL || op == NOT_EQ) {
		if (!is_num_type(t1) || !is_num_type(t1)) {
			cout << "Несоответствие типов" << endl;
			throw c;
		}
		ret = BOOL;
	} else if (op == MOD) {
		if (!is_int_type(t1) || !is_int_type(t2)) {
			cout << "Несоответствие типов операндов" << endl;
			throw c;
		}
		if (t1 == ULL || t2 == ULL) {
			ret = ULL;
		} else if (t1 == LL || t2 == LL) {
			ret = LL;
		} else if (t1 == UINT || t2 == UINT) {
			ret = UINT;
		} else if (t1 == INT || t2 == INT) {
			ret = INT;
		} else if (t1 == USHORT || t2 == USHORT) {
			ret = USHORT;
		} else if (t1 == SHORT || t2 == SHORT) {
			ret = SHORT;
		} else if (t1 == UCHAR || t2 == UCHAR) {
			ret = UCHAR;
		} else if (t1 == CHAR || t2 == CHAR) {
			ret = CHAR;
		} else {
			cout << "Несоответствие типов" << endl;
			throw c;
		}
		
	} else if (op == ASSIGN) {
		if (t2 == STR) {
			cout << "Не могу присвоить строчку в эту переменную" << endl;
			throw c;
		}
		ret = t1;
	} else {
		cout << "Неизвестная операция " << op << endl;
		throw c;
	}
	push(ret);
}

void add_area() {
	tids.push_back(new hashtable);
}

bool add_var(var * a) {
	return tids.back()->insert(a);
}

VARTYPE find_var(string a) {
	for (int i = tids.size()-1; i >= 0; i--) {
		var * v = tids[i]->get(a);
		if (v != nullptr) {
			//cout << a << endl;
			push(v->type);
			return v->type;
		}
	}
	return NONE;
}

var * get_var(string a) {
	for (int i = tids.size()-1; i >= 0; i--) {
		var * v = tids[i]->get(a);
		if (v != nullptr) {
			return v;
		}
	}
	return nullptr;
}

void remove_tid() {
	if (tids.size() == 0) {
		cout << "ОПА" << endl;
		return;
	}
	delete tids.back();
	tids.pop_back();
}

bool operator == (token t, string s) {
	return t.value == s;
}

bool operator == (token t, TYPE s) {
	return t.type == s;
}

bool operator != (token t, string s) {
	return t.value != s;
}

bool operator != (token t, TYPE s) {
	return t.type != s;
}

bool is_composite_type(token & c) {
	return c.type == RESERVED && (c.value == "long" || c.value == "short" || c.value == "signed" || c.value == "unsigned");
}

bool is_type(token & c) {
	return (c.type == RESERVED && (is_composite_type(c) || c.value == "int" || c.value == "double" || c.value == "float" || c.value == "char" || c.value == "bool"));
}

bool is_comparison_op(token & c) {
	return (c.type == OPERATION) && (c.value == ">" || c.value == "<" || c.value == "!=" || c.value == "==" || c.value == "<=" || c.value == ">="); 
}

bool is_sum_op(token & c) {
	return c.type == OPERATION && (c.value == "+" || c.value == "-" || c.value == "||");
}

bool is_mult_op(token & c) {
	return c.type == OPERATION && (c.value == "*" || c.value == "/" || c.value == "&&" || c.value == "%"); 
}

VARTYPE convert(token & c) {
	if (c == "+") return SUM;
	if (c == "-") return SUB;
	if (c == "||") return OR;
	if (c == "*") return MULT;
	if (c == "/") return DIV;
	if (c == "&&") return AND;
	if (c == "!") return NOT;
	if (c == "%") return MOD;
	if (c == ">") return MORE;
	if (c == "<") return LESS;
	if (c == ">=") return MORE_EQ;
	if (c == "<=") return LESS_EQ;
	if (c == "==") return EQUAL;
	if (c == "!=") return NOT_EQ;
	if (c == "^") return POW;
	if (c == "=") return ASSIGN;
	if (c == "++") return INC;
	if (c == "--") return DEC;
	if (c == CONSTCHAR) return CHAR;
	if (c == CONSTINT) return INT;
	if (c == CONSTLOGIC) return BOOL;
	if (c == CONSTREAL) return DOUBLE;
	if (c == CONSTSTR) return STR;
	return NONE;
}

struct poliz_elem {
	VARTYPE op = NONE;
	//string value = "";
	var * addr = nullptr;
	int lnk = 0;
	poliz_elem (VARTYPE t, var * a) : op(t), addr(a) {};
	poliz_elem (VARTYPE t, int l) : op(t), lnk(l) {};
	poliz_elem (VARTYPE t) : op(t) {};
	poliz_elem () {};
};

vector<poliz_elem> polska;

int poliz_add_const(VARTYPE t, string value) {
	var * a = new var("const", t);
	a->set_value(value);
	polska.push_back({ADDRESS, a});
	return polska.size()-1;
}

int poliz_add_op(VARTYPE t) {
	polska.push_back({t});
	return polska.size()-1;
}

int poliz_add_link(int l) {
	polska.push_back({LINK, l});
	return polska.size()-1;
}

int poliz_add_var(var * a) {
	polska.push_back({ADDRESS, a});
	return polska.size()-1;
}

vector<token> list;
size_t last_elem = 0;

set <string> reserved = {"int", "long", "short", "char", "float", "double", "void", "signed", "unsigned", "bool", "return",
	"continue", "for", "while", "do", "if", "else", "break", "true", "false", "to", "downto"};

void lexic_analis (char * infile, vector<token> &list) {
	ifstream fin(infile);
	int state = 0;
	token current;
	current.value = "";
	current.line = 1;
	current.position = 0;
	char c;
	while (fin.get(c)) {
		if (c == '\n') {
			current.line++;
			current.position = 0;
		} else {
			current.position++;
		}
		if (state == 0) {
			if (c == ' ') continue;
			else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') { 
				state = 1;
				current.value += c;
			} else if ('0' <= c && c <= '9') {
				state = 2;
				current.value += c;
			} else if (c == '"') {
				state = 6;
			} else if (c == '\'') {
				state = 8;
			} else if (c == '(' || c == ')') {
				current.value += c;
				current.type = BRACKETS;
				list.push_back(current);
				current.value = "";
				current.type = UNDEF;
			} else if (c == '{' || c == '}' || c == ';' || c == ':' || c == '[' || c == ']' || c == ',') {
				current.value += c;
				current.type = PUNCTUATION;
				list.push_back(current);
				current.value = "";
				current.type = UNDEF;
			} else if (c == '*' || c == '/' || c == '+' || c == '-' || c == '=' || c == '%' || c == '&' || c == '.' ||
					   c == '!' || c == '~' || c == '<' || c == '>' || c == '^' || c == '|') {
				fin.unget();
				current.position--;
				state = 11;
			}
		} else if (state == 1) {//Идентификатор или зарезервированное слово;
			if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') {
				current.value += c;
			} else {
				fin.unget();
				current.position--;
				if (reserved.count(current.value) == 1) {
					if (current.value == "true" || current.value == "false") {
						current.type = CONSTLOGIC;
					} else {
						current.type = RESERVED;
					}
				} else {
					current.type = ID;
				}
				list.push_back(current);
				current.value = "";
				current.type = UNDEF;
				state = 0;
			}
		} else if (state == 2) {//целая часть числа;
			if ('0' <= c && c <= '9') {
				current.value += c;
			} else if (c == '.') {
				current.value += c;
				state = 3;
			} else if (c == 'e' || c == 'E') {
				current.value += c;
				state = 4;
			} else {
				fin.unget();
				current.position--;
				current.type = CONSTINT;
				list.push_back(current);
				current.type = UNDEF;
				current.value = "";
				state = 0;
			}
		} else if (state == 3) {//Число после десятичной точки;
			if ('0' <= c && c <= '9') {
				current.value += c;
			} else if (c == 'e'|| c == 'E') {
				current.value += c;
				state = 4;
			} else if (c == '.') {
				throw "Две десятичных точки в одном числе";
			} else {
				fin.unget();
				current.position--;
				current.type = CONSTREAL;
				list.push_back(current);
				current.type = UNDEF;
				current.value = "";
				state = 0;
			}
		} else if (state == 4) {//Число после символа e;
			if (c == '+' || c == '-' || ('0' <= c && c <= '9')) {
				current.value += c;
				state = 5;
			} else {
				throw "Не обозначен порядок числа";
			}
		} else if (state == 5) {//число со знаком порядка;
			if ('0' <= c && c <= '9') {
				current.value += c;
			} else if (c == 'e' || c == 'E') {
				throw "Два символа начала порядка";
			} else if (c == '.') {
				throw "Две десятичных точки в одном числе";
			} else {
				fin.unget();
				current.position--;
				current.type = CONSTREAL;
				list.push_back(current);
				current.type = UNDEF;
				current.value = "";
				state = 0;
			}
		} else if (state == 6) {//Начали считывать строку;
			if (c == '"') {
				current.type = CONSTSTR;
				list.push_back(current);
				current.type = UNDEF;
				current.value = "";
				state = 0;
			} else if (c == '\\') {
				state = 7;
			} else {
				current.value += c;
			}
		} else if (state == 7) {//обработка escape-последовательностей в строке;
			if (c == 'n') {
				current.value += '\n';
			} else if (c == 't') {
				current.value += '\t';
			} else if (c == 'r') {
				current.value += '\r';
			} else if (c == 'v') {
				current.value += '\v';
			} else if (c == 'a') {
				current.value += '\a';
			} else if (c == 'b') {
				current.value += '\b';
			} else if (c == 'f') {
				current.value += '\f';
			} else if (c == '\n' || c == '\r') {
				if (fin.get(c) && c != '\n' && c == '\r') {
					fin.unget();
					current.position--;
				}
			} else {
				current.value += c;
			}
			state = 6;
		} else if (state == 8) {//Начали обрабатывать сивольную константу;
			if (c == '\\') {
				state = 9;
			} else if (c == '\'') {
				throw "Пустая символьная константа";
			} else {
				current.value = c;
				state = 10;
			}
		} else if (state == 9) {//обработка escape-последовательности в символе;
			if (c == 'n') {
				current.value += '\n';
			} else if (c == 't') {
				current.value += '\t';
			} else if (c == 'r') {
				current.value += '\r';
			} else if (c == 'v') {
				current.value += '\v';
			} else if (c == 'a') {
				current.value += '\a';
			} else if (c == 'b') {
				current.value += '\b';
			} else if (c == 'f') {
				current.value += '\f';
			} else {
				current.value += c;
			}
			state = 10;
		} else if (state == 10) {//Окончание символьной константы;
			if (c == '\'') {
				current.type = CONSTCHAR;
				list.push_back(current);
				current.type = UNDEF;
				current.value = "";
				state = 0;
			} else {
				throw "Многознаковая символьная константа";
			}
		} else if (state == 11) {//Надо считать оператор;
			if (c == '+') {
				current.value += "+";
				if (fin.get(c)) {
					current.position++;
					if (c == '+' || c == '=') {
						current.value += c;
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '-') {
				current.value += "-";
				if (fin.get(c)) {
					current.position++;
					if (c == '-' || c == '=' || c == '>') {
						current.value += c;
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '&') {
				current.value += c;
				if (fin.get(c)) {
					current.position++;
					if (c == '=' || c == '&') {
						current.value += c;
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '|') {
				current.value += c;
				if (fin.get(c)) {
					current.position++;
					if (c == '=' || c == '|') {
						current.value += c;
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '/') {
				current.value += c;
				if (fin.get(c)) {
					current.position++;
					if (c == '=' || c == '|') {
						current.value += c;
					} else if (c == '/') {
						current.value = "";
						state = 12;
						continue;
					} else if (c == '*') {
						current.value = "";
						state = 13;
						continue;
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '=' || c == '%' || c == '!' || c == '*' || c == '^') {
				current.value += c;
				if (fin.get(c)) {
					current.position++;
					if (c == '=') {
						current.value += c;
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '<') {
				current.value += c;
				if (fin.get(c)) {
					current.position++;
					if (c == '=') {
						current.value += c;
					} else if (c == '<') {
						current.value += c;
						if (fin.get(c)) {
							current.position++;
							if (c == '=') {
								current.value += c;
							} else {
								fin.unget();
								current.position--;
							}
						}
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '>') {
				current.value += c;
				if (fin.get(c)) {
					current.position++;
					if (c == '=') {
						current.value += c;
					} else if (c == '>') {
						current.value += c;
						if (fin.get(c)) {
							current.position++;
							if (c == '=') {
								current.value += c;
							} else {
								fin.unget();
								current.position--;
							}
						}
					} else {
						fin.unget();
						current.position--;
					}
				}
			} else if (c == '.' || c == '~') {
				current.value += c;
			}
			current.type = OPERATION;
			list.push_back(current);
			current.type = UNDEF;
			current.value = "";
			state = 0;
		} else if (state == 12) {//Читаем однострочный комментарий
			if (c == '\n') {
				state = 0;
			}
		} else if (state == 13) {//Читаем многострочный комментарий
			if (c == '*') {
				if (fin.get(c)) {
					current.position++;
					if (c == '/') {
						state = 0;
					}
				}
			}
		}
	}
	current.type = UNDEF;
	current.value = "END";
	list.push_back(current);
}

void gc() {
	if (last_elem < list.size()) c = list[last_elem++];
	else c = list.back();
}

void program();
void description();//доедает за собой ;
void composite_operator();
void section(VARTYPE t);//запятые не ест.
void expression(); //запятыми не кормить
void single_operator();
void input_operator();
void output_operator();
void choose_for();
void dowhile();
void whiledo();
void simple_expr();
void term();
void atom1();
bool atom();
void cfor();
void pfor();

void program() {
	//cout << "program " << c.value << endl;
	add_area();
	while (c.type != UNDEF) {
		if (c.type == RESERVED) {
			if (c.value == "int") {
				gc();
				if (c.value == "main") {
					gc();
					if (c.value != "("){
						cout << "После main ожидалась скобка" << endl;
						throw c;
					}
					gc();
					if (c.value != ")") {
						cout << "Не хватает закрывающей скобки после main" << endl;
						throw c;
					}
					gc();
					add_area();
					composite_operator();
					remove_tid();
					break;
				} else {
					last_elem-=2;
					gc();
					description();
				}
			} else {
				description();
			}
		} else {
			cout << "Тут вообще что-то левое." << endl;
			throw c;
		};
	}
	remove_tid();
	if (c != UNDEF) {
		cout << "Тут вообще что-то левое." << endl;
		throw c;
	}
}

void description() {//доедает свою ;
	//cout << "desc " << c.value << endl;
	if (c.type != RESERVED) {
		cout << "Ожидалось название типа" << endl;
		throw c;
	}
	VARTYPE t = NONE;
	if (c == "signed") {
		gc();
		if (c == "char") {
			t = CHAR;
			gc();
		} else if (c == "short") {
			t = SHORT;
			gc();
			if (c == "int") {
				gc();
			}
		} else if (c == "int") {
			t = INT;
			gc();
		} else if (c == "long") {
			t = INT;
			gc();
			if (c == "int") {
				gc();
			} else if (c == "long") {
				t = LL;
				gc();
				if (c == "int") {
					gc();
				}
			}
		}
	} else if (c == "unsigned") {
		gc();
		if (c == "char") {
			t = UCHAR;
			gc();
		} else if (c == "short") {
			t = USHORT;
			gc();
			if (c == "int") {
				gc();
			}
		} else if (c == "int") {
			t = UINT;
			gc();
		} else if (c == "long") {
			t = UINT;
			gc();
			if (c == "int") {
				gc();
			} else if (c == "long") {
				t = ULL;
				gc();
				if (c == "int") {
					gc();
				}
			}
		}
	} else if (c == "short") {
		t = SHORT;
		gc();
		if (c == "int") {
			gc();
		}
	} else if (c == "long") {
		t = INT;
		gc();
		if (c == "int") {
			gc();
		} else if (c == "long") {
			t = LL;
			gc();
			if (c == "int") {
				gc();
			}
		} else if (c == "double") {
			t = LDOUBLE;
			gc();
		}
	} else if (c == "char") {
		t = CHAR;
		gc();
	} else if (c == "int") {
		t = INT;
		gc();
	} else if (c == "float") {
		t = FLOAT;
		gc();
	} else if (c == "double") {
		t = DOUBLE;
		gc();
	} else if (c == "bool") {
		t = BOOL;
		gc();
	} else {
		cout << "У Вас типы сломались" << endl;
		throw c;
	}
	section(t);
	while (c.value == ",") {
		gc();
		section(t);
	}
	if (c.value != ";") {
		cout << "Ожидалась ; после описания" << endl;
		throw c;
	}
	gc();
}

void section(VARTYPE t) { //запятые и ; не ест.
	//cout << t << endl;
	//cout << "section " << c.value << endl;
	if (c.type != ID) {
		cout << "Ожидалось имя переменной для описания" << endl;
		throw c;
	}
	var * a = new var(c.value, t);
	gc();
	if (c.value == "," || c.value == ";") {
		if (add_var(a) == 0) {
			cout << "Переменная " << a->name << " объявлена ранее" << endl;
			delete a;
			throw c;
		}
		return;
	}
	if (c.value == "=") {
		gc();
		int p = poliz_add_var(nullptr);
		expression();// возможно, присвоить в a полученное значение
		if (add_var(a) == 0) {
			cout << "Переменная " << a->name << " объявлена ранее" << endl;
			delete a;
			throw c;
		}
		VARTYPE tt = st.back();
		st.pop_back();
		if (tt == STR) {
			cout << "Ну не могу я строки присваивать.." << endl;
			throw c;
		}
		polska[p].addr = get_var(a->name);
		poliz_add_op(ASSIGN);
		poliz_add_op(SEMICOLON);
	} else {
		cout << "Ожидалась ; после описания или инициализация переменной" << endl;
		throw c;
	}
}

void composite_operator() { //Доедает за собой свои скобки {}
	//cout << "cmp_op " << c.value << endl;
	if (c != "{") {
		cout << "Ожидалась { " << endl;
		throw c;
	}
	gc();
	while (c != "}" && c != UNDEF) {
		single_operator();
	}
	if (c == UNDEF) {
		cout << "Внезапный конец файла" << endl;
		throw c;
	};
	gc();
}

void expression() {//запятые и ; не ест.
	//cout << "expr " << c.value << endl;
	if (c == ID) {
		var * t = get_var(c.value);
		if (t == nullptr) {
			cout << "Использована необъявленная переменная " << c.value << endl;
			throw c;
		}
		gc();
		if (c == "=") {
			push(t->type);
			push(convert(c));
			poliz_add_var(t);
			gc();
			expression();
			check_op();
			poliz_add_op(ASSIGN);
		} else {
			last_elem-=2;
			gc();
			//st.pop_back();
			goto suda;
		}
	} else {
		suda:;
		simple_expr();
		while (is_comparison_op(c)) {
			VARTYPE op = convert(c);
			push(op);
			gc();
			simple_expr();
			check_op();
			poliz_add_op(op);
		}
	}
}

void simple_expr() {
	//cout << "s_expr " << c.value << endl;
	term();
	while(is_sum_op(c)){
		VARTYPE op = convert(c);
		push(op);
		gc();
		term();
		check_op();
		poliz_add_op(op);
	}
}

void term() {
	//cout << "term " << c.value << endl;
	atom1();
	while(is_mult_op(c)) {
		VARTYPE op = convert(c);
		push(op);
		gc();
		atom1();
		check_op();
		poliz_add_op(op);
	}
}

void atom1() {
	//cout << "atom1 " << c.value << endl;
	bool a = atom();
	if (c == "++" || c == "--") {
		if (!a) {
			cout << "Использована операция " << c.value << " не для переменной"<< endl;
			throw c;
		}
		if (!is_num_type(st.back())) {
			cout << "Операция " << c.value << " не применима к этому типу" << endl;
		}
		poliz_add_op(convert(c));
		gc();
		return;
	}
	if (c == "^") {
		VARTYPE op = convert(c);
		push(op);
		gc();
		atom();
		check_op();
		poliz_add_op(op);
		return;
	}
}

bool atom() {//возвращает 1, если это была просто переменная.
	//cout << "atom " << c.value << endl;
	if (c == ID) {
		var * t = get_var(c.value);
		if (t == nullptr) {
			cout << "Использована необъявленная переменная " << c.value << endl;
			throw c;
		}
		push(t->type);
		poliz_add_var(t);
		gc();
		return 1;
	} else if (c == CONSTCHAR || c == CONSTINT || c == CONSTLOGIC || c == CONSTREAL || c == CONSTSTR) {
		push(convert(c));
		poliz_add_const(convert(c), c.value);
		gc();
		return 0;
	}
	if (c == "!" || c == "+" || c == "-") {//унарный
		VARTYPE op = convert(c);
		push(op);
		if (op == SUM) op = UN_SUM;
		if (op == SUB) op = UN_SUB;
		gc();
		atom();
		check_un();
		poliz_add_op(op);
		return 0;
	}
	if (c == "(") {
		gc();
		expression();
		if (c != ")") {
			cout << "А ну закрыл скобку, быстраа!!" << endl;
			throw c;
		};
		gc();
		return 0;
	}
	cout << "Неизвестный науке оператор" << endl;
	throw c;
}

void single_operator() {//доедает за собой ; но не }
	//cout << "operator" << ' ' << c.value << endl;
	if (c == "cin") {
		gc();
		input_operator();
		return;
	}
	if (c == "cout") {
		gc();
		output_operator();
		return;
	}
	if (c == "{") {
		composite_operator();
		return;
	}
	if (c == "for") {
		gc();
		choose_for();
		return;
	}
	if (c == "do") {
		gc();
		dowhile();
		return;
	}
	if (c == "while") {
		gc();
		whiledo();
		return;
	}
	if (is_type(c)) {
		description();
		return;
	}
	if (c == "return") {
		gc();
		expression();
		poliz_add_op(SEMICOLON);
		VARTYPE t = st.back();
		st.pop_back();
		if (!is_int_type(t)) {
			cout << "Не могу такого вернуть" << endl;
			throw c;
		}
		if (c != ";") {
			cout << "Ожидалась ; после return" << endl;
			throw c;
		};
		gc();
		return;
	}
	expression();//А не доесть ли мне здесь ; ?
	if (c == ";") {
		poliz_add_op(SEMICOLON);
		gc();
		st.pop_back();
	}
}

void input_operator() {
	//cout << "input " << c.value << endl;
	if (c != ">>") {
		cout << "Нормальные люди написали бы здесь >> " << endl;
		throw c;
	};
	var * t;
	ss:;
	gc();
	if (c != ID) {
		cout << "Я думал, ты в переменную считывать собираешься..." << endl;
		throw c;
	};
	t = get_var(c.value);
	//st.pop_back();
	if (t == nullptr) {
		cout << "Использована необъявленная переменная " << c.value << endl;
		throw c;
	}
	poliz_add_var(t); 
	poliz_add_op(CIN);
	gc();
	if (c == ">>") goto ss;
	if (c != ";") {
		cout << "Опять забываешь ; ?" << endl;
		throw c;
	}
	gc();
}

void output_operator() {
	//cout << "output " << c.value << endl;
	if (c != "<<") {
		cout << "Нормальные люди написали бы здесь <<" << endl;
		throw c;
	}
	gc();
	while (true) {
		if (c != "endl") {
			expression();
			st.pop_back();
			poliz_add_op(COUT);
		}
		else gc();
		if (c == ";") {
			gc();
			break;
		}
		if (c != "<<") {
			cout << "Опять ; забываешь?" << endl;
			throw c;
		}
		gc();
	}
}

void choose_for() {
	//cout << "choose_for " << c.value << endl;
	add_area();
	if (c != "(") {
		cout << "Я надеюсь, ты забываешь скобочки в for не намеренно?" << endl;
		throw c;
	}
	gc();
	if (c == ID) {
		VARTYPE t = find_var(c.value);
		if (t == NONE) {
			cout << "Использована необъявленная переменная " << c.value << endl;
			throw c;
		}
		gc();
		if (c == ":") {
			gc();
			if (c == "=") {
				last_elem-=3;
				gc();
				pfor();
			} else {
				cout << "Ты наконец решишь, Паскаль я или не Паскаль? Ставь тут либо :=, либо пиши нормально." << endl;
				throw c;
			}
		} else {
			last_elem -= 2;
			gc();
			cfor();
		}
	} else {
		cfor();
	}//началось вырезание
	if (c == "else") {
		gc();
		single_operator();
	}//до сюда
	remove_tid();
}

void cfor () {
	var * b = new var("hidden", BOOL);
	poliz_add_var(b);
	poliz_add_const(BOOL, "false");//доделать константы
	poliz_add_op(ASSIGN);
	poliz_add_op(SEMICOLON);
	if (is_type(c)) {
		description();
	} else {
		expression();
		st.pop_back();
		if (c != ";") {
			cout << "Серьезно? Перестань забывать ; , пожалуйста!" << endl;
			throw c;
		}
		poliz_add_op(SEMICOLON);
		gc();
	}
	int a3 = polska.size();
	expression();
	VARTYPE t = st.back();
	st.pop_back();
	if (t != BOOL) {
		cout << "Условие в for возвращает не bool" << endl;
		throw c;
	}
	if (c != ";") {
		cout << "МНЕ НУЖНА ; Я НЕ ПИТОН! ХОТЯ, ЕСЛИ ЕЩЕ ПАРУ РАЗ ЗАБУДЕШЬ, КУСАТЬСЯ НАЧНУ..." << endl;
		throw c;
	}
	int pa1 = poliz_add_link(0);
	poliz_add_op(FALSE_MOV);
	int pa2 = poliz_add_link(0);
	poliz_add_op(MOV);
	int a4 = polska.size();
	gc();
	expression();
	st.pop_back();
	if (c != ")") {
		cout << "Осторожно, скобки закрываются" << endl;
		throw c;
	}
	poliz_add_op(SEMICOLON);
	poliz_add_link(a3);
	poliz_add_op(MOV);
	poliz_add_var(b);
	poliz_add_const(BOOL, "true");//доделать константы
	poliz_add_op(ASSIGN);
	poliz_add_op(SEMICOLON);
	polska[pa2].lnk = polska.size();
	gc();
	single_operator();
	poliz_add_link(a4);
	poliz_add_op(MOV);
	polska[pa1].lnk = polska.size();	
	if (c == "else") {
		poliz_add_var(b);
		int pa5 = poliz_add_link(0);
		poliz_add_op(FALSE_MOV);
		gc();
		single_operator();
		polska[pa5].lnk = polska.size();
	}
}

void pfor() {
	//cout << "pfor " << c.value << endl;
	if (c != ID) {
		cout << "Тот самый случай, когда ошибка в компиляторе" << endl;
		throw c;
	}
	gc();
	if (c != ":") {
		cout << "Тот самый случай, когда ошибка в компиляторе" << endl;
		throw c;
	}
	gc();
	if (c != "=") {
		cout << "Тот самый случай, когда ошибка в компиляторе" << endl;
		throw c;
	}
	gc();
	push(ASSIGN);
	expression();
	check_op();
	if (c == "to" || c == "downto") {
		push(ASSIGN);
		gc();
		expression();
		check_op();
		st.pop_back();
		if (c != ")") {
			cout << "Скобки закрывай! Ты не в метро!" << endl;
			throw c;
		}
		gc();
		single_operator();
	} else {
		cout << "Вверх или вниз?" << endl;
		throw c;
	}
}

void dowhile() {
	add_area();
	//cout << "do while " << c.value << endl;
	int a2 = polska.size();
	single_operator();
	if (c != "while") {
		cout << "А как же while?" << endl;
		throw c;
	}
	gc();
	if (c != "(") {
		cout << "Хм... Вы не ставите скобки в while... Это жжжжж неспроста..." << endl;
		throw c;
	}
	gc();
	expression();
	VARTYPE t = st.back();
	st.pop_back();
	if (t != BOOL) {
		cout << "Условие в do while возвращает не bool" << endl;
		throw c;
	}
	
	int p = poliz_add_link(0);
	poliz_add_op(FALSE_MOV);
	poliz_add_link(a2);
	poliz_add_op(MOV);
	polska[p].lnk = polska.size();
	if (c != ")") {
		cout << "Прикрой скобку, мне дует!" << endl;
		throw c;
	}
	gc();
	if (c != ";") {
		cout << "Нет, я все понимаю, но не мог бы ты все-таки поставить сюда ; ?" << endl;
		throw c;
	}
	remove_tid();
	gc();
}

void whiledo() {
	add_area();
	//cout << "while do " << c.value << endl;
	if (c != "(") {
		cout << "Товарищ, не забывайте, пожалуйста, ставить скобки в операторах цикла." << endl;
		throw c;
	}
	gc();
	int a2 = polska.size();
	expression();
	VARTYPE t = st.back();
	st.pop_back();
	if (t != BOOL) {
		cout << "Условие в while возвращает не bool" << endl;
		throw c;
	}
	if (c != ")") {
		cout << "Сегодня ты не закрыл скобку... А завтра не закроешь сессию?" << endl;
		throw c;
	}
	int p = poliz_add_link(0);
	poliz_add_op(FALSE_MOV);
	gc();
	single_operator();
	poliz_add_link(a2);
	poliz_add_op(MOV);
	polska[p].lnk = polska.size();
	remove_tid();
}

int main(int argc, char ** argv) {
	char infile[256] = {0};
	char outfile[256] = {0};
	bool o = 0;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'o') {
				o = 1;
			}
		}
		else {
			if (!o && infile[0] == 0) {
				char * a = &argv[i][strlen(argv[i])-1];
				if (argv[i][0] == '\'' && *a == '\'') {
					*a = 0;
					strcpy(infile, argv[i]+1);
				} else {
					strcpy(infile, argv[i]);
				}
			} else if (o && outfile[0] == 0) {
				char * a = &argv[i][strlen(argv[i])-1];
				if (*a == '\'') {
					*a = 0;
					strcpy(outfile, argv[i]+1);
				} else {
					strcpy(outfile, argv[i]);
				}
				o = 0;
			} else {
				if (o) {
					if (outfile[0] == 0) {
						cout << "Отсутствует файл для вывода" << endl;
					} else {
						cout << "Указано несколько файлов для вывода" << endl;
					}
				}
				else {
					if (infile[0] == 0) {
						cout << "Отсутствует исходный файл" << endl;
					} else {
						cout << "Указано несколько исходных файлов" << endl;
					}
				}
				return 0;
			}
		}
	}
	if (infile[0] == 0)	{
		cout << "Не указан исходный файл" << endl;
		return 0;
	}
	if (outfile[0] == 0) {
		cout << "Не указан файл для вывода" << endl;
		return 0;
	}
	try {
	lexic_analis(infile, list);
	} catch (string s) {
		cout << s << endl;
		return 0;
	}
	last_elem = 0;
	try {
		gc();
		program();
	} catch (token a) {
		cout << "Строка " << a.line << ", позиция " << a.position << endl;
		cout << "Плохой символ: " << a.type << ' ' << a.value << endl;
		return 1;
	}
	cout << "Я, конечно, не эксперт, но вроде все нормально" << endl;
	for (int i = 0; i < polska.size(); i++) {
		cout << i << ' ' << polska[i].op << " ";
		if (polska[i].op == ADDRESS) {
			cout << polska[i].addr->name;
		}
		if (polska[i].op == SUM) cout << "+";
		if (polska[i].op == SUB) cout << "-";
		if (polska[i].op == MULT) cout << "*";
		if (polska[i].op == DIV) cout << "/";
		if (polska[i].op == MOD) cout << "%";
		if (polska[i].op == POW) cout << "^";
		if (polska[i].op == INC) cout << "++";
		if (polska[i].op == SEMICOLON) cout << ";";
		if (polska[i].op == ASSIGN) cout << "=";
		if (polska[i].op == CIN) cout << "CIN";
		if (polska[i].op == COUT) cout << "COUT";
		if (polska[i].op == LINK) cout << "LINK " << polska[i].lnk;
		if (polska[i].op == MOV) cout << "! ";
		if (polska[i].op == FALSE_MOV) cout << "!f ";
		cout << endl;
	}
	/*
	for (size_t i = 0; i < list.size(); i++) {
		cout << list[i].type << ' ' << list[i].line << ' ' << list[i].position ;
		
		if (list[i].type == BRACKETS) cout << "(Скобки)";
		else if (list[i].type == CONSTCHAR) cout << "(Символьная константа)";
		else if (list[i].type == CONSTINT) cout << "(Целочисленная константа)";
		else if (list[i].type == CONSTSTR) cout << "(Строковая константа)";
		else if (list[i].type == CONSTREAL) cout << "(Действительная константа)";
		else if (list[i].type == CONSTLOGIC) cout << "(Логическая константа)";
		else if (list[i].type == ID) cout << "(Идентификатор)";
		else if (list[i].type == OPERATION) cout << "(Операция)";
		else if (list[i].type == PUNCTUATION) cout << "(Пунктуация)";
		else if (list[i].type == RESERVED) cout << "(Зарезервированное слово)";
		else if (list[i].type == UNDEF) cout << "(Не определено)";
		cout << ' ' << list[i].value << endl;
	}/**/
	return 0;
} 
