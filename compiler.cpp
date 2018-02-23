#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cstdio>
#include <set>
using namespace std;

enum TYPE{UNDEF, RESERVED, ID, CONSTINT, CONSTREAL, CONSTSTR, CONSTCHAR, CONSTLOGIC, OPERATION, PUNCTUATION, BRACKETS};

struct token {
	TYPE type = UNDEF;
	string value;
	int line;
	int position;
};

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
	return c.type == RESERVED && (c.value == "long" || c.value == "signed" || c.value == "unsigned");
}

bool is_type(token & c) {
	return (c.type == RESERVED && (c.value == "int" || c.value == "double" || c.value == "float" || c.value == "char" || c.value == "bool"));
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

vector<token> list;
size_t last_elem = 0;

set <string> reserved = {"int", "long", "char", "float", "double", "void", "signed", "unsigned", "bool", "return", "continue",
	"for", "while", "do", "if", "else", "break", "true", "false", "to", "downto"};

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

token c;

void gc() {
	if (last_elem < list.size()) c = list[last_elem++];
	else c = list.back();
}

void program();
void description();//доедает за собой ;
void composite_operator();
void section();//запятые не ест.
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
void atom();
void cfor();
void pfor();

void program() {
	//cout << "program " << c.value << endl;
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
					composite_operator();
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
	if (c.value == "long" || c.value == "signed" || c.value == "unsigned") {
		gc();
		if (c.value == "int" || c.value == "double" || c.value == "float" || c.value == "char" || c.value == "bool") {
			goto ss;
		} else {
			last_elem--;
			gc();
			goto ss;
		}
	}
	if (c.value == "int" || c.value == "double" || c.value == "float" || c.value == "char" || c.value == "bool") {
		ss:;
		gc();
		section();
		while (c.value == ",") {
			gc();
			section();
		}
		if (c.value != ";") {
			cout << "Ожидалась ; после описания" << endl;
			throw c;
		}
		gc();
		return;
	}
	cout << "Кажется, у вас неправильные типы" << endl;
	throw c;
}

void section() { //запятые и ; не ест.
	//cout << "section " << c.value << endl;
	if (c.type != ID) {
		cout << "Ожидалось имя переменной для описания" << endl;
		throw c;
	}
	gc();
	if (c.value == "," || c.value == ";") return;
	if (c.value == "=") {
		gc();
		expression();
	} else {
		cout << "Ожидалась ; после описания или инициализация переменной" << endl;
		throw c;
	}
}

void composite_operator() { //Доедает за собой свои скобки {}
	//cout << "cmp_op " << c.value << endl;
	if (c != "{") {
		cout << "Ожидалась } " << endl;
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
		gc();
		if (c == "=") {
			gc();
			expression();
		} else {
			last_elem-=2;
			gc();
			goto suda;
		}
	} else {
		suda:;
		simple_expr();
		while (is_comparison_op(c)) {
			gc();
			simple_expr();
		}
	}
}

void simple_expr() {
	//cout << "s_expr " << c.value << endl;
	term();
	while(is_sum_op(c)){
		gc();
		term();
	}
}

void term() {
	//cout << "term " << c.value << endl;
	atom1();
	while(is_mult_op(c)) {
		gc();
		atom1();
	}
}

void atom1() {
	//cout << "atom1 " << c.value << endl;
	atom();
	if (c == "++" || c == "--") {
		gc();
		return;
	}
	if (c == "^") {
		gc();
		atom();
		return;
	}
}

void atom() {
	//cout << "atom " << c.value << endl;
	if (c == ID || c == CONSTCHAR || c == CONSTINT || c == CONSTLOGIC || c == CONSTREAL || c == CONSTSTR){
		gc();
		return;
	}
	if (c == "+" || c == "-") {
		gc();
		if (c == ID || c == CONSTCHAR || c == CONSTINT || c == CONSTREAL) {
			gc();
			return;
		}
		if (c == "(") goto bracket;
		cout << "Унарный оператор " << c.value << " не применим к данному типу" << endl;
		throw c;
	}
	if (c == "!") {
		gc();
		atom();
		return;
	}
	if (c == "(") {
		bracket:;
		gc();
		expression();
		if (c != ")") {
			cout << "А ну закрыл скобку, быстраа!!" << endl;
			throw c;
		};
		gc();
		return;
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
	if (is_composite_type(c) || is_type(c)) {
		description();
		return;
	}
	if (c == "return") {
		gc();
		expression();
		if (c != ";") {
			cout << "Ожидалась ; после return" << endl;
			throw c;
		};
		gc();
		return;
	}
	expression();//А не доесть ли мне здесь ; ?
	if (c == ";") gc();
}

void input_operator() {
	//cout << "input " << c.value << endl;
	if (c != ">>") {
		cout << "Нормальные люди написали бы здесь >> " << endl;
		throw c;
	};
	ss:;
	gc();
	if (c != ID) {
		cout << "Я думал, ты в переменную считывать собираешься..." << endl;
		throw c;
	};
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
		if (c != "endl") expression();
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
	if (c != "(") {
		cout << "Я надеюсь, ты забываешь скобочки в for не намеренно?" << endl;
		throw c;
	}
	gc();
	if (c == ID) {
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
	}
	if (c == "else") {
		gc();
		single_operator();
	}
}

void cfor () {
	if (is_type(c) || is_composite_type(c)) {
		description();
	} else {
		expression();
		if (c != ";") {
			cout << "Серьезно? Перестань забывать ; , пожалуйста!" << endl;
			throw c;
		}
		gc();
	}
	expression();
	if (c != ";") {
		cout << "МНЕ НУЖНА ; Я НЕ ПИТОН! ХОТЯ, ЕСЛИ ЕЩЕ ПАРУ РАЗ ЗАБУДЕШЬ, КУСАТЬСЯ НАЧНУ..." << endl;
		throw c;
	}
	gc();
	expression();
	if (c != ")") {
		cout << "Осторожно, скобки закрываются" << endl;
		throw c;
	}
	gc();
	single_operator();
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
	expression();
	if (c == "to" || c == "downto") {
		gc();
		expression();
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
	//cout << "do while " << c.value << endl;
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
	if (c != ")") {
		cout << "Прикрой скобку, мне дует!" << endl;
		throw c;
	}
	gc();
	if (c != ";") {
		cout << "Нет, я все понимаю, но не мог бы ты все-таки поставить сюда ; ?" << endl;
		throw c;
	}
	gc();
}

void whiledo() {
	//cout << "while do " << c.value << endl;
	if (c != "(") {
		cout << "Товарищ, не забывайте, пожалуйста, ставить скобки в операторах цикла." << endl;
		throw c;
	}
	gc();
	expression();
	if (c != ")") {
		cout << "Сегодня ты не закрыл скобку... А завтра не закроешь сессию?" << endl;
		throw c;
	}
	gc();
	single_operator();
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
		return 0;
	}
	cout << "Я, конечно, не эксперт, но вроде все нормально" << endl;
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
