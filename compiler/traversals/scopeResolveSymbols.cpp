#include <typeinfo>
#include "scopeResolveSymbols.h"
#include "expr.h"
#include "stmt.h"
#include "stringutil.h"
#include "symtab.h"
#include "../traversals/fixup.h"


static void check_legal_overload(Symbol* sym) {
  if (sym->overload) {
    int count = 0;
    for (Symbol* tmp = sym; tmp; tmp = tmp->overload) {
      if (!tmp->getFnSymbol()) {
        count++;
      }
    }
    if (count >= 2) {
      char* redefinitionLocations = "";
      for (Symbol* tmp = sym->overload; tmp; tmp = tmp->overload) {
        if (!tmp->getFnSymbol()) {
          redefinitionLocations =
            glomstrings(3, redefinitionLocations, "\n  ", tmp->stringLoc());
        }
      }
      USR_FATAL(sym, "'%s' has multiple definitions, redefined at:%s",
                sym->name, redefinitionLocations);
    }
  }
}


ScopeResolveSymbols::ScopeResolveSymbols() {
  defList = new Map<SymScope*,Vec<VarSymbol*>*>();
}


void ScopeResolveSymbols::postProcessExpr(Expr* expr) {
  if (Variable* sym_use = dynamic_cast<Variable*>(expr)) {
    if (sym_use->var->isUnresolved) {
      SymScope* currentScope = Symboltable::getCurrentScope();
      char* name = sym_use->var->name;
      
      if (!strcmp(name, "__primitive")) {
        return;
      }

      VarSymbol* sym_in_scope =
        dynamic_cast<VarSymbol*>(Symboltable::lookupInCurrentScope(name));
      
      if (sym_in_scope) {
        Vec<VarSymbol*>* sym_defs = defList->get(currentScope);
        if (!sym_defs || !sym_defs->set_in(sym_in_scope)) {
          USR_FATAL(expr, "Variable '%s' used before it is defined", name);
        }
      }
      
      Symbol* sym_resolve = Symboltable::lookup(name);
      
      if (VarSymbol* var_resolve = dynamic_cast<VarSymbol*>(sym_resolve)) {
        ModuleSymbol* mod =
          dynamic_cast<ModuleSymbol*>(var_resolve->parentScope->symContext);
        if (mod && currentScope == mod->initFn->body->blkScope) {
          Vec<VarSymbol*>* sym_defs = defList->get(currentScope);
          if (!sym_defs || !sym_defs->set_in(var_resolve)) {
            USR_FATAL(expr, "Variable '%s' used before it is defined", name);
          }
        }
      }

      if (sym_resolve) {
        check_legal_overload(sym_resolve);
        if (!dynamic_cast<FnSymbol*>(sym_resolve)) {
          if (ForwardingSymbol* forward =
              dynamic_cast<ForwardingSymbol*>(sym_resolve)) {
            if (!dynamic_cast<FnSymbol*>(forward->forward)) {
              sym_use->var = forward->forward;
              sym_use->forward = forward;
            }
          } else {
            sym_use->var = sym_resolve;
          }
        }
      } else {
        USR_FATAL(expr, "Symbol '%s' is not defined", name);
      }
    }
  }

  if (DefExpr* def_expr = dynamic_cast<DefExpr*>(expr)) {
    if (VarSymbol* sym = dynamic_cast<VarSymbol*>(def_expr->sym)) {
      Vec<VarSymbol*>* syms = defList->get(Symboltable::getCurrentScope());
      if (syms) {
        syms->set_add(sym);
      } else {
        syms = new Vec<VarSymbol*>();
        syms->set_add(sym);
        defList->put(Symboltable::getCurrentScope(), syms);
      }
    }
  }

  if (DefExpr* def_expr = dynamic_cast<DefExpr*>(expr)) {
    if (FnSymbol* fn = dynamic_cast<FnSymbol*>(def_expr->sym)) {
      if (fn->retType == dtUnknown &&
          def_expr->exprType &&
          def_expr->exprType->typeInfo() != dtUnknown) {
        fn->retType = def_expr->exprType->typeInfo();
        def_expr->exprType = NULL;
      }
    } else if (def_expr->sym->type == dtUnknown &&
               def_expr->exprType &&
               def_expr->exprType->typeInfo() != dtUnknown) {
      def_expr->sym->type = def_expr->exprType->typeInfo();
      def_expr->exprType = NULL;
    }
  }

  if (CastExpr* castExpr = dynamic_cast<CastExpr*>(expr)) {
    if (castExpr->type == dtUnknown &&
        castExpr->newType &&
        castExpr->newType->typeInfo() != dtUnknown) {
      castExpr->type = castExpr->newType->typeInfo();
      castExpr->newType = NULL;
    }
  }
}
