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
};

set <string> reserved = {"int", "long", "char", "float", "double", "void", "signed", "unsigned", "bool", "return", "continue",
	"for", "while", "do", "if", "else", "break", "true", "false", };

void lexic_analis (char * infile, vector<token> &list) {
	ifstream fin(infile);
	int state = 0;
	token current;
	current.value = "";
	char c;
	while (fin.get(c)) {
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
				state = 11;
			}
		} else if (state == 1) {//Идентификатор или зарезервированное слово;
			if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') {
				current.value += c;
			} else {
				fin.unget();
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
					if (c == '+' || c == '=') {
						current.value += c;
					} else {
						fin.unget();
					}
				}
			} else if (c == '-') {
				current.value += "-";
				if (fin.get(c)) {
					if (c == '-' || c == '=' || c == '>') {
						current.value += c;
					} else {
						fin.unget();
					}
				}
			} else if (c == '&') {
				current.value += c;
				if (fin.get(c)) {
					if (c == '=' || c == '&') {
						current.value += c;
					} else {
						fin.unget();
					}
				}
			} else if (c == '|') {
				current.value += c;
				if (fin.get(c)) {
					if (c == '=' || c == '|') {
						current.value += c;
					} else {
						fin.unget();
					}
				}
			} else if (c == '/') {
				current.value += c;
				if (fin.get(c)) {
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
					}
				}
			} else if (c == '=' || c == '%' || c == '!' || c == '*' || c == '^') {
				current.value += c;
				if (fin.get(c)) {
					if (c == '=') {
						current.value += c;
					} else {
						fin.unget();
					}
				}
			} else if (c == '<') {
				current.value += c;
				if (fin.get(c)) {
					if (c == '=') {
						current.value += c;
					} else if (c == '<') {
						current.value += c;
						if (fin.get(c)) {
							if (c == '=') {
								current.value += c;
							} else {
								fin.unget();
							}
						}
					} else {
						fin.unget();
					}
				}
			} else if (c == '>') {
				current.value += c;
				if (fin.get(c)) {
					if (c == '=') {
						current.value += c;
					} else if (c == '>') {
						current.value += c;
						if (fin.get(c)) {
							if (c == '=') {
								current.value += c;
							} else {
								fin.unget();
							}
						}
					} else {
						fin.unget();
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
					if (c == '/') {
						state = 0;
					}
				}
			}
		}
	}
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
	vector<token> list;
	lexic_analis(infile, list);
	for (int i = 0; i < list.size(); i++) {
		cout << list[i].type << ' ' ;
		
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
	}
	return 0;
} 
