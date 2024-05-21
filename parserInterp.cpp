/* Implementation of Recursive-Descent Parser
 * for the SFort95 Language
 * parser(SP24).cpp
 * Programming Assignment 2
 * Spring 2024
*/

#include "parserInterp.h"

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects
namespace Parser {
    bool pushed_back = false;
    LexItem    pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if( pushed_back ) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem & t) {
        if( pushed_back ) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;
}

bool IdentList(istream& in, int& line);


bool Prog(istream& in, int& line)
{
    bool dl = false, sl = false;
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok.GetToken() == PROGRAM) {
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == IDENT) {
            string identstr = tok.GetLexeme();
            auto it = defVar.find(identstr);

            if (it != defVar.end()) {
                if (!it->second) {
                    defVar[identstr] = true;
                }
            } else {
                defVar.insert({tok.GetLexeme(), true}); // if is not declared, insert the variable into defVar and return true
            }
            

            dl = Decl(in, line);
            if (!dl) {
                ParseError(line, "Incorrect Declaration in Program");
                return false;
            }
            sl = Stmt(in, line);
            if (!sl) {
                ParseError(line, "Incorrect Statement in program");
                return false;
            }
            tok = Parser::GetNextToken(in, line);

            if (tok.GetToken() == END) {
                tok = Parser::GetNextToken(in, line);

                if (tok.GetToken() == PROGRAM) {
                    tok = Parser::GetNextToken(in, line);

                    if (tok.GetToken() == IDENT) {
                        if (tok.GetLexeme() != identstr) {
                            ParseError(line, "Mismatching program name at the end");
                            return false;
                        }
                        cout << "(DONE)" << endl;
                        return true;
                    } else {
                        ParseError(line, "Missing Program Name");
                        return false;
                    }
                } else {
                    ParseError(line, "Missing PROGRAM at the End");
                    return false;
                }
            } else {
                ParseError(line, "Missing END of Program");
                
                return false;
            }
        } else {
            ParseError(line, "Missing Program Name");
            return false;
        }
    } else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    return true;
}


//Decl ::= Type :: VarList
bool Decl(istream& in, int& line) {
    bool status = false;
    LexItem tokType;
    string strLen = "1"; // Default string length for characters

    LexItem tok = Parser::GetNextToken(in, line);
    
    if(tok == INTEGER || tok == REAL || tok == CHARACTER) {
        tokType = tok; // Save the type token

        // Handling potential length specification for CHARACTER type
        if (tok == CHARACTER) {
            tok = Parser::GetNextToken(in, line);
            if (tok == LPAREN) { // Check for optional string length
                tok = Parser::GetNextToken(in, line);
                if (tok == LEN) {
                    tok = Parser::GetNextToken(in, line);
                    if (tok == ASSOP) {
                        tok = Parser::GetNextToken(in, line);
                        if (tok == ICONST) {
                            strLen = tok.GetLexeme(); // Set string length if specified
                            tok = Parser::GetNextToken(in, line);
                            if (tok != RPAREN) {
                                ParseError(line, "Missing Right Parenthesis after String Length.");
                                return false;
                            }
                        } else {
                            ParseError(line, "Expected Integer after Assignment in String Length declaration.");
                            return false;
                        }
                    } else {
                        ParseError(line, "Expected Assignment Operator after LEN.");
                        return false;
                    }
                } else {
                    ParseError(line, "Expected LEN for String Length declaration.");
                    return false;
                }
                tok = Parser::GetNextToken(in, line); // Expecting DCOLON after RPAREN
            }
            if (tok != DCOLON) {
                ParseError(line, "Missing Double Colon after Length specification.");
                return false;
            }
        }
        // Process the variable list
        status = VarList(in, line, tokType, stoi(strLen));

        if (!status) {
            ParseError(line, "Error in Variable List.");
            return false;
        }

        // Recursively process subsequent declarations
        if (!Decl(in, line)) {
            return false;
        }
    }

    return true;
}

//VarList ::= Var [= Expr] {, Var [= Expr]
bool VarList(istream& in, int& line, LexItem &idtok, int strLen) {
    bool status = false, exprstatus = false;
    string identstr;
    Value val1, val2; // val2 is used if there's an initialization expression
    LexItem tok;

    tok = Parser::GetNextToken(in, line);
    if(tok == IDENT) {
        identstr = tok.GetLexeme();
        if (!defVar[identstr]) {
            defVar[identstr] = true;
            if(idtok.GetToken() == DCOLON) {
                SymTable[identstr] = CHARACTER;
                string spaces = "";
                spaces.append(strLen, ' ');
                TempsResults[identstr] = Value(spaces);
                TempsResults[identstr].SetstrLen(strLen);
            } else {
                SymTable[identstr] = idtok.GetToken();
            }
        } else {
            ParseError(line, "Variable Redefinition");
            return false;
        }
    }

    tok = Parser::GetNextToken(in, line);
    if(tok == ASSOP) {
        exprstatus = Expr(in, line, val2);
        if (!exprstatus) {
            ParseError(line, "Incorrect initialization for variable.");
            return false;
        }

        // Additional logic based on variable type and initialization
        if (idtok == REAL && val2.GetType() == VINT) {
            // Conversion from int to real before assignment if needed
            double r = static_cast<double>(val2.GetInt());
            TempsResults[identstr] = Value(r);
        } else if (SymTable[identstr] == CHARACTER) {
            string sv = val2.GetString();
            if (sv.length() > strLen) {
                sv = sv.substr(0, strLen);
            } else if (sv.length() < strLen) {
                sv.append(strLen - sv.length(), ' ');
            }
            TempsResults[identstr] = Value(sv);
            TempsResults[identstr].SetstrLen(sv.length());
        } else {
            TempsResults[identstr] = val2;
        }
    } else {
        Parser::PushBackToken(tok);
    }

    tok = Parser::GetNextToken(in, line);
    if(tok == COMMA) {
        status = VarList(in, line, idtok, strLen);
    } else {
        Parser::PushBackToken(tok);
        status = true;
    }
    return status;
}



//Stmt ::= AssigStmt | BlockIfStmt | PrintStmt | SimpleIfStmt
bool Stmt(istream& in, int& line) {
    bool status;
    
    LexItem t = Parser::GetNextToken(in, line);
    
    switch( t.GetToken() ) {

    case PRINT:
        status = PrintStmt(in, line);
        
        if(status)
            status = Stmt(in, line);
        break;

    case IF:
        status = BlockIfStmt(in, line);
        if(status)
            status = Stmt(in, line);
        break;

    case IDENT:
        Parser::PushBackToken(t);
        status = AssignStmt(in, line);
        if(status)
            status = Stmt(in, line);
        break;
        
    
    default:
        Parser::PushBackToken(t);
        return true;
    }

    return status;
}//End of Stmt
bool PrintStmt(istream& in, int& line) {
    LexItem t;
    ValQue = new queue<Value>;
    
    
    t = Parser::GetNextToken(in, line);
    
     if( t != DEF ) {
        
        ParseError(line, "Print statement syntax error.");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    
    if( t != COMMA ) {
        
        ParseError(line, "Missing Comma.");
        return false;
    }
    bool ex = ExprList(in, line);
    
    if( !ex ) {
        ParseError(line, "Missing expression after Print Statement");
        return false;
    }
    
    while (!(*ValQue).empty())
    {
        Value nextVal = (*ValQue).front();
        cout << nextVal;
        ValQue->pop();
    }
    cout << endl;
    return ex;
}//End of PrintStmt

//BlockIfStmt:= if (Expr) then {Stmt} [Else Stmt]
//SimpleIfStatement := if (Expr) Stmt
bool BlockIfStmt(istream& in, int& line) {
    bool ex=false, status ;
    LexItem t;
    Value val1;
    static int nestlevel = 0;
    int level;
    
    
    t = Parser::GetNextToken(in, line);
    if( t != LPAREN ) {
        
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    
    ex = RelExpr(in, line,val1);
    if( !ex ) {
        ParseError(line, "Missing if statement condition");
        return false;
    }
    
    t = Parser::GetNextToken(in, line);
    if(t != RPAREN ) {
        
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    
    t = Parser::GetNextToken(in, line);
    if(t != THEN)
    {
        Parser::PushBackToken(t);
        
        status = SimpleStmt(in, line);
        if(status)
        {
            return true;
        }
        else
        {
            ParseError(line, "If-Stmt Syntax Error");
            return false;
        }
        
    }
    if (val1.GetType() != VBOOL)
    {
        ParseError(line, "Run-Time Error-Illegal Type for If statement condition");
        return false;
    }
    nestlevel++;
    level = nestlevel;
    status = Stmt(in, line);
    
    if(!status)
    {
        ParseError(line, "Missing Statement for If-Stmt Then-Part");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if( t == ELSE ) {
        status = Stmt(in, line);
        if(!status)
        {
            ParseError(line, "Missing Statement for If-Stmt Else-Part");
            return false;
        }
        else
         
          t = Parser::GetNextToken(in, line);
        
        
    }
    
    
    if(t != END ) {
        
        ParseError(line, "Missing END of IF");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if(t == IF ) {
        cout << "End of Block If statement at nesting level " << level << endl;
        return true;
    }
    
    Parser::PushBackToken(t);
    ParseError(line, "Missing IF at End of IF statement");
    return false;
}//End of IfStmt function
// SimpleIfStatement := if (Expr) Stmt
bool SimpleIfStmt(istream& in, int& line) {
    LexItem t;
    Value val1;
    bool exprResult;

    // Expect 'IF' token already consumed by the caller
    
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis after IF");
        return false;
    }
    
    exprResult = RelExpr(in, line, val1); // Evaluate the relational expression
    if (!exprResult) {
        ParseError(line, "Invalid or missing condition in IF statement");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis after IF condition");
        return false;
    }

    if (val1.GetType() != VBOOL) {
        ParseError(line, "IF statement condition must be a boolean");
        return false;
    }

    // Directly proceed to the statement part after the IF condition
    bool status = Stmt(in, line);
    if (!status) {
        ParseError(line, "Error in statement following IF");
        return false;
    }

    return true;
}

bool SimpleStmt(istream& in, int& line) {
    bool status;
    
    LexItem t = Parser::GetNextToken(in, line);
    
    switch( t.GetToken() ) {

    case PRINT:
        status = PrintStmt(in, line);
        
        if(!status)
        {
            ParseError(line, "Incorrect Print Statement");
            return false;
        }
        cout << "Print statement in a Simple If statement." << endl;
        break;

    case IDENT:
        Parser::PushBackToken(t);
        status = AssignStmt(in, line);
        if(!status)
        {
            ParseError(line, "Incorrect Assignent Statement");
            return false;
        }
        cout << "Assignment statement in a Simple If statement." << endl;
            
        break;
        
    
    default:
        Parser::PushBackToken(t);
        return true;
    }

    return status;
}//End of SimpleStmt

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
    bool varstatus = false, status = false;
    LexItem t;
    Value val1, val2;
    string identstr;
    varstatus = Var(in, line,t);
    //Value retVal(VERR);
    t = Parser::GetNextToken(in, line);

    if (varstatus) {

        identstr = t.GetLexeme();
        if (SymTable[identstr] == BCONST)
            val1.SetType(VBOOL);
        else if(SymTable[identstr] == ICONST)
            val1.SetType(VINT);
        else if (SymTable[identstr] == RCONST)
            val1.SetType(VREAL);
        else if (SymTable[identstr] == ICONST )
            val1.SetType(VSTRING);


        t = Parser::GetNextToken(in, line);

        if (t == ASSOP) {
            status = Expr(in, line, val2);//--------------------------
            if (!status) {
                ParseError(line, "Missing Expression in Assignment Statement");
                return status;
            }

        }
        else if (t.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
        }
        else {
            ParseError(line, "Missing Assignment Operator");
            return false;
        }
    }
    else {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
        return false;
    }
    if (val1.GetType() == VREAL)
    {
        if (val2.GetType() == VINT)
        {
            //val2.SetType(VREAL);

            val1.SetReal((float)(val2.GetInt()));
            TempsResults[identstr] = val1;
            return status;
        }
        if (val2.GetType() == VSTRING)
        {
            ParseError(line, "Illegal mixed-mode assignment operation");
            return false;
        }
        TempsResults[identstr] = val2;
    }
    if (val1.GetType() == VINT)
    {
        if (val2.GetType() == VREAL)
        {
            val1.SetInt(int(val2.GetReal()));
            TempsResults[identstr] = val1;
            return status;
        }
        if (val2.GetType() == VSTRING)
        {
            ParseError(line, "STRING DIFERENT OF INT wrong assignment");
            return false;
        }
        TempsResults[identstr] = val2;
    }
    if (val1.GetType() == VSTRING)
    {
        if (val2.GetType() == VINT || val2.GetType() == VREAL)
        {

            ParseError(line, "Illegal mixed-mode assignment operation");
            return false;
        }
        TempsResults[identstr] = val2;
    }

    if (val1.GetType() == VBOOL)
    {
        if (val2.GetType() == VINT || val2.GetType() == VREAL || val2.GetType() == VSTRING)
        {

            ParseError(line, "Illegal mixed-mode assignment operation");
            return false;
        }
        TempsResults[identstr] = val2;
    }


    return status;
}
//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    Value retVal;
    
    status = Expr(in, line, retVal);
    if(!status){
        ParseError(line, "Missing Expression");
        return false;
    }
    ValQue->push(retVal);
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok == COMMA) {
        
        status = ExprList(in, line);
        
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}//End of ExprList
//RelExpr ::= SimpleExpr  [ ( = | < | > ) SimpleExpr ]
bool RelExpr(istream& in, int& line, Value& retVal) {
    LexItem tok;
    Value val1, val2;
    LexItem t;
    t = Parser::GetNextToken(in, line);
    Parser::PushBackToken(t);
    bool t1 = Expr(in, line, val1);

    if (!t1) {
        //retVal.SetType(VERR);
        return false;
    }
    retVal = val1;
    
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        //retVal.SetType(VERR);
        return false;
    }
    
    if (tok == EQ)
    {
        t1 = Expr(in, line, val2);
        if (!t1)
        {
            ParseError(line, "Missing expression after relational operator");
            return false;
        }
        retVal = val1 == val2;
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Run-Time Error-Illegal Mixed Type operation");
            return false;
        }
    //    if (retVal.GetBool())
            return true;
        //else
            //return false;
    }
    else if (tok == LTHAN)//||tok == LTHAN
    {
        t1 = Expr(in, line, val2);
        if (!t1)
        {
            ParseError(line, "Missing expression after relational operator");
            return false;
        }

        retVal = val1 < val2;
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Missing expression after relational operator");
            return false;
        }
    //    if (retVal.GetBool())
            return true;
        //else
        //    return false;
    }
    else if (tok == GTHAN)//
    {

    t = Parser::GetNextToken(in, line);
    Parser::PushBackToken(t);
         t1 = Expr(in, line, val2);
        if (!t1)
        {
            ParseError(line, "Missing expression after relational operator");
            return false;
        }
        retVal = val1 > val2;
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Missing expression after relational operator");
            return false;
        }
    //    if (retVal.GetBool())
            return true;
        //else
        //    return false;
    }
    
    Parser::PushBackToken(tok);
    return true;
}//End of RelExpr

//Expr ::= MultExpr { ( + | - | // ) MultExpr }
bool Expr(istream& in, int& line,Value& retVal) {
    Value val1,val2;
    LexItem t;
    t = Parser::GetNextToken(in, line);
    Parser::PushBackToken(t);
    bool t1 = MultExpr(in, line,val1);
    LexItem tok;
    
    if( !t1 ) {
        return false;
    }
    retVal=val1;
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while ( tok == PLUS || tok == MINUS || tok == CAT)
    {
        t1 = MultExpr(in, line,val2);
        if( !t1 )
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (tok==CAT)
        {
            if(!(retVal.GetType() == VSTRING || val2.GetType() == VSTRING)){
                ParseError(line, "Run-Time Error-Illegal Mixed Type Operands");
                return false;
            }
            else{
                retVal=retVal + val2;
            }
        }
        else if (!(retVal.GetType() == VSTRING || val2.GetType() == VSTRING))
        {
            if(tok==PLUS){
                retVal=retVal + val2;
            }
            else if(tok==MINUS){
                retVal=retVal - val2;
            }
            
        }
        tok = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
        
    }
    Parser::PushBackToken(tok);
    return true;
}//End of Expr



//MultExpr ::= TermExpr { ( * | / ) TermExpr }
bool MultExpr(istream& in, int& line,Value& retVal) {
    Value val1,val2;
    
    bool t1 = TermExpr(in, line,retVal);
    LexItem tok;
    
    if( !t1 ) {
        return false;
    }
    retVal=val1;
    tok    = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
    }
    while ( tok == MULT || tok == DIV  )
    {
        t1 = TermExpr(in, line,retVal);
        if(tok==MULT)
            retVal=retVal* val1;
        else {
            if (val1.GetInt() == 0)
            {
                ParseError(line, "Run-Time Error-Illegal division by Zero");
                return false;
            }
            else {
                retVal = retVal / val1;
            }
        
        }
        if( !t1 ) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Illegal operand types for an arithmetic operator");
            return false;
        }
        tok    = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
        
    }
    Parser::PushBackToken(tok);
    return true;
}//End of MultExpr
//TermExpr ::= SFactor { ** SFactor }
bool TermExpr(istream& in, int& line, Value& retVal) {
    Value val1,val2;
    bool t1 = SFactor(in, line,val1);
    LexItem tok;

    if( !t1 ) {
        return false;
    }
    retVal=val1;
    tok    = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
    }
    while ( tok == POW  )
    {
        t1 = SFactor(in, line,val1);
        retVal = retVal * val1;
        
        if( !t1 ) {
            ParseError(line, "Missing exponent operand");
            return false;
        }
        if (retVal.GetType() == VERR)
        {
            ParseError(line, "Illegal operand types for an arithmetic operator");
            return false;
        }

        tok    = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
        
    }
    Parser::PushBackToken(tok);
    return true;
}//End of TermExpr
bool Factor(istream& in, int& line, Value& retVal,int sign) {

    LexItem tok = Parser::GetNextToken(in, line);


    if (tok == IDENT) {
        string lexeme = tok.GetLexeme();
        if (!(defVar.find(lexeme)->second))
        {
            ParseError(line, "Using Undefined Variable");
            //retVal.SetType(VERR);
            return false;
        }
        retVal = TempsResults[lexeme];
        if (TempsResults[lexeme].GetType() == VERR) {
            ParseError(line, "Using uninitialized Variable");
            return false;
        }
        if ((sign == -1|| sign==1)  && (retVal.GetType() == VSTRING ))
        {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator");
            return false;
        }
        
        if (retVal.GetType() == VINT)
            retVal.SetInt(sign * retVal.GetInt());
        if (retVal.GetType() == VREAL)
            retVal.SetReal(sign * retVal.GetReal());
        
        return true;
    }
    else if (tok == ICONST) {
        
        string lexeme = tok.GetLexeme();
        double temp = stoi(lexeme);

        retVal.SetType(VINT);
        retVal.SetInt(sign * temp);
        return true;
    }
    else if (tok == SCONST) {
        retVal.SetType(VSTRING);
        retVal.SetString( tok.GetLexeme());
        return true;
    }
    else if (tok == RCONST) {
        retVal.SetType(VREAL);
        retVal.SetReal(sign * stof(tok.GetLexeme()));
        return true;
    }
    
    else if (tok == LPAREN) {
        bool ex = Expr(in, line,retVal);
        if (!ex) {
            ParseError(line, "Missing expression after (");
            //retVal.SetType(VERR);
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok == RPAREN)
        {
            return ex;
        }
        else
        {
            Parser::PushBackToken(tok);
            ParseError(line, "Missing ) after expression");
            //retVal.SetType(VERR);
            return false;
        }
    }
    else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        //retVal.SetType(VERR);
        return false;
    }
    Parser::PushBackToken(tok);
    //retVal.SetType(VERR);
    return false;
}


//SFactor = Sign Factor | Factor
bool SFactor(istream& in, int& line,Value &retVal)
{
    LexItem t = Parser::GetNextToken(in, line);
    
    bool status;
    int sign = 1;
    if(t == MINUS )
    {
        sign = -1;
    }
    else if(t == PLUS)
    {
        sign = 1;
    }
    else
        Parser::PushBackToken(t);
        
    status =Factor(in, line,retVal, sign);
    return status;
}//End of SFactor
//Var = ident
bool Var(istream& in, int& line, LexItem& idtok)
{
    string identstr;

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == IDENT) {
        identstr = tok.GetLexeme();

        if (!(defVar.find(identstr)->second))
        {
            ParseError(line, "Undeclared Variable");
            return false;
        }
        Parser::PushBackToken(tok);
        return true;
    }
    else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    return false;
}//End of Var






