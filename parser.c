/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 * Refactored version
 */

#include <stdlib.h>
#include <stdio.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

/* Global token management */
Token *currentToken;
Token *lookAhead;

/* ========== Token Management Functions ========== */

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else {
    missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
  }
}

/* ========== Helper Functions ========== */

static int isComparisonOperator(TokenType type) {
  return (type == SB_EQ || type == SB_NEQ || 
          type == SB_LT || type == SB_LE || 
          type == SB_GT || type == SB_GE);
}

static int isAddOperator(TokenType type) {
  return (type == SB_PLUS || type == SB_MINUS);
}

static int isMultOperator(TokenType type) {
  return (type == SB_TIMES || type == SB_SLASH);
}

static int isBasicType(TokenType type) {
  return (type == KW_INTEGER || type == KW_CHAR);
}

static int isSubroutineDecl(TokenType type) {
  return (type == KW_FUNCTION || type == KW_PROCEDURE);
}

static void eatCommaIdentList(void) {
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
  }
}

/* ========== Program & Block Compilation ========== */

void compileProgram(void) {
  assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  assert("Program parsed!");
}

void compileBlock(void) {
  assert("Parsing a Block ....");
  
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
  }
  
  compileBlock2();
  assert("Block parsed!");
}

void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
  }
  
  compileBlock3();
}

void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
  }
  
  compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

/* ========== Declaration Compilation ========== */

void compileConstDecls(void) {
  if (lookAhead->tokenType == TK_IDENT) {
    compileConstDecl();
    compileConstDecls();
  }
}

void compileConstDecl(void) {
  eat(TK_IDENT);
  eatCommaIdentList();
  eat(SB_EQ);
  compileConstant();
  eat(SB_SEMICOLON);
}

void compileTypeDecls(void) {
  if (lookAhead->tokenType == TK_IDENT) {
    compileTypeDecl();
    compileTypeDecls();
  }
}

void compileTypeDecl(void) {
  eat(TK_IDENT);
  eatCommaIdentList();
  eat(SB_EQ);
  compileType();
  eat(SB_SEMICOLON);
}

void compileVarDecls(void) {
  if (lookAhead->tokenType == TK_IDENT) {
    compileVarDecl();
    compileVarDecls();
  }
}

void compileVarDecl(void) {
  eat(TK_IDENT);
  eatCommaIdentList();
  eat(SB_COLON);
  compileType();
  eat(SB_SEMICOLON);
}

void compileSubDecls(void) {
  assert("Parsing subtoutines ....");
  
  while (isSubroutineDecl(lookAhead->tokenType)) {
    if (lookAhead->tokenType == KW_FUNCTION) {
      compileFuncDecl();
    } else {
      compileProcDecl();
    }
  }
  
  assert("Subtoutines parsed ....");
}

void compileFuncDecl(void) {
  assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Function parsed ....");
}

void compileProcDecl(void) {
  assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Procedure parsed ....");
}

/* ========== Type & Constant Compilation ========== */

void compileUnsignedConstant(void) {
  if (lookAhead->tokenType == TK_NUMBER || lookAhead->tokenType == TK_CHAR) {
    eat(lookAhead->tokenType);
  } else {
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileConstant(void) {
  if (isAddOperator(lookAhead->tokenType)) {
    eat(lookAhead->tokenType);
  }
  compileUnsignedConstant();
}

void compileConstant2(void) {
  switch (lookAhead->tokenType) {
    case TK_IDENT:
    case TK_NUMBER:
      eat(lookAhead->tokenType);
      break;
    default:
      error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileType(void) {
  switch (lookAhead->tokenType) {
    case KW_INTEGER:
    case KW_CHAR:
    case TK_IDENT:
      eat(lookAhead->tokenType);
      break;
      
    case KW_ARRAY:
      eat(KW_ARRAY);
      eat(SB_LSEL);
      eat(TK_NUMBER);
      eat(SB_RSEL);
      eat(KW_OF);
      compileType();
      break;
      
    default:
      error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileBasicType(void) {
  if (isBasicType(lookAhead->tokenType)) {
    eat(lookAhead->tokenType);
  } else {
    error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
  }
}

/* ========== Parameter Compilation ========== */

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    compileParams2();
    eat(SB_RPAR);
  }
}

void compileParams2(void) {
  if (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileParam();
    compileParams2();
  }
}

void compileParam(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
  }
  
  if (lookAhead->tokenType == TK_IDENT) {
    eat(TK_IDENT);
    eat(SB_COLON);
    compileBasicType();
  } else {
    error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
  }
}

/* ========== Statement Compilation ========== */

void compileStatements(void) {
  compileStatement();
  compileStatements2();
}

void compileStatements2(void) {
  switch (lookAhead->tokenType) {
    case SB_SEMICOLON:
      eat(SB_SEMICOLON);
      compileStatement();
      compileStatements2();
      break;
      
    case KW_END:
    case KW_UNTIL:
      break;
      
    default:
      eat(SB_SEMICOLON);
      error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
    case TK_IDENT:
      compileAssignSt();
      break;
    case KW_CALL:
      compileCallSt();
      break;
    case KW_BEGIN:
      compileGroupSt();
      break;
    case KW_IF:
      compileIfSt();
      break;
    case KW_WHILE:
      compileWhileSt();
      break;
    case KW_FOR:
      compileForSt();
      break;
    case KW_REPEAT:
      compileRepeatSt();
      break;
      
    /* Empty statement - check FOLLOW tokens */
    case SB_SEMICOLON:
    case KW_END:
    case KW_ELSE:
    case KW_UNTIL:
      break;
      
    default:
      error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileAssignSt(void) {
  assert("Parsing an assign statement ....");
  
  eat(TK_IDENT);
  int countVariables = 1;
  int countExpression = 0;
  
  if (lookAhead->tokenType == SB_LSEL) {
    compileIndexes();
  }
  
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
    countVariables++;
  }
  
  eat(SB_ASSIGN);
  compileExpression();
  countExpression++;
  
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
    countExpression++;
  }
  
  if (countExpression != countVariables) {
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    return;
  }
  
  assert("Assign statement parsed ....");
}

void compileCallSt(void) {
  assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
  assert("Call statement parsed ....");
}

void compileGroupSt(void) {
  assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  assert("Group statement parsed ....");
}

void compileIfSt(void) {
  assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  
  if (lookAhead->tokenType == KW_ELSE) {
    compileElseSt();
  }
  
  assert("If statement parsed ....");
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  assert("While statement pased ....");
}

void compileForSt(void) {
  assert("Parsing a for statement ....");
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  assert("For statement parsed ....");
}

void compileRepeatSt(void) {
  assert("Parsing a repeat statement ....");
  eat(KW_REPEAT);
  compileStatements();
  eat(KW_UNTIL);
  compileCondition();
  assert("Repeat statement parsed ....");
}

/* ========== Expression & Condition Compilation ========== */

void compileArguments(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileExpression();
    compileArguments2();
    eat(SB_RPAR);
  }
}

void compileArguments2(void) {
  switch (lookAhead->tokenType) {
    case SB_COMMA:
      eat(SB_COMMA);
      compileExpression();
      compileArguments2();
      break;
    case SB_RPAR:
      break;
    default:
      error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileCondition(void) {
  compileExpression();
  compileCondition2();
}

void compileCondition2(void) {
  if (isComparisonOperator(lookAhead->tokenType)) {
    eat(lookAhead->tokenType);
    compileExpression();
  }
}

void compileExpression(void) {
  assert("Parsing an expression");
  
  if (isAddOperator(lookAhead->tokenType)) {
    eat(lookAhead->tokenType);
  }
  
  compileExpression2();
  assert("Expression parsed");
}

void compileExpression2(void) {
  compileTerm();
  compileExpression3();
}

void compileExpression3(void) {
  if (isAddOperator(lookAhead->tokenType)) {
    eat(lookAhead->tokenType);
    compileTerm();
    compileExpression3();
  }
}

void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  if (isMultOperator(lookAhead->tokenType)) {
    eat(lookAhead->tokenType);
    compileFactor();
    compileTerm2();
  }
}

void compileFactor(void) {
  switch (lookAhead->tokenType) {
    case TK_NUMBER:
    case TK_CHAR:
      eat(lookAhead->tokenType);
      break;
      
    case TK_IDENT:
      eat(TK_IDENT);
      if (lookAhead->tokenType == SB_LSEL) {
        compileIndexes();
      } else if (lookAhead->tokenType == SB_LPAR) {
        compileArguments();
      }
      break;
      
    case SB_LPAR:
      eat(SB_LPAR);
      compileExpression();
      eat(SB_RPAR);
      break;
      
    default:
      error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileIndexes(void) {
  if (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
    compileIndexes();
  }
}

/* ========== Main Compile Function ========== */

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR) {
    return IO_ERROR;
  }

  currentToken = NULL;
  lookAhead = getValidToken();

  /* Redirect token output to result.txt */
  if (freopen("result.txt", "w", stdout) == NULL) {
    fprintf(stderr, "Warning: cannot open result.txt for writing\n");
  }

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  
  return IO_SUCCESS;
}