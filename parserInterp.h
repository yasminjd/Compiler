/* 
 * parserInterp.h
 * Programming Assignment 3
 * Spring 2024
*/

#ifndef PARSER_H_
#define PARSER_H_

#include <iostream>

using namespace std;

#include "lex.h"
#include "val.h"
//1)Prog ::= PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
extern bool Prog(istream& in, int& line);
//2)Decl ::= Type :: VarList
extern bool Decl(istream& in, int& line);
//3)Type ::= INTEGER | REAL | CHARACTER [(LEN = ICONST)]
extern bool Type(istream& in, int& line);
//4)VarList ::= Var [= Expr] {, Var [= Expr]}
//in parser.h://extern bool VarList(istream& in, int& line);
//LexItem & idtok: can add or look entries in the symbol table
//keep track of variable types for all defined variables
//if (undeclared usage , redeclaration , used for providing error messages )
extern bool VarList(istream& in, int& line, LexItem & idtok, int strlen = 1 );

extern bool Stmt(istream& in, int& line);
extern bool SimpleStmt(istream& in, int& line);
extern bool PrintStmt(istream& in, int& line);
extern bool BlockIfStmt(istream& in, int& line);
extern bool SimpleIfStmt(istream& in, int& line);
extern bool AssignStmt(istream& in, int& line);

//17. Var ::= IDENT
//extern bool Var(istream& in, int& line);
extern bool Var(istream& in, int& line, LexItem & idtok);

extern bool ExprList(istream& in, int& line);

//12. RelExpr ::= Expr [ ( == | < | > ) Expr ]
//extern bool RelExpr(istream& in, int& line);
extern bool RelExpr(istream& in, int& line, Value & retVal);
//13. Expr ::= MultExpr { ( + | - | // ) MultExpr }
//extern bool Expr(istream& in, int& line);
extern bool Expr(istream& in, int& line, Value & retVal);
//14. MultExpr ::= TermExpr { ( * | / ) TermExpr }
//extern bool MultExpr(istream& in, int& line);
extern bool MultExpr(istream& in, int& line, Value & retVal);
//15. TermExpr ::= SFactor { ** SFactor }
//extern bool TermExpr(istream& in, int& line);
extern bool TermExpr(istream& in, int& line, Value & retVal);

//SFactor ::= [+ | -] Facto
//extern bool SFactor(istream& in, int& line);
extern bool SFactor(istream& in, int& line, Value & retVal);
//Factor ::= IDENT | ICONST | RCONST | SCONST | (Expr)
//extern bool Factor(istream& in, int& line, int sign);
extern bool Factor(istream& in, int& line, int sign, Value & retVal);

extern int ErrCount();

#endif /* PARSER_H_ */
