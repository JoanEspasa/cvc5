#include <cvc5/cvc5.h>

#include <iostream>

using namespace cvc5;

static void tryLogic(const char* logic)
{
  TermManager tm;
  Solver slv(tm);
  try
  {
    slv.setLogic(logic);
    Term d = tm.mkTerm(tm.mkOp(Kind::PLAN_DOES, {0, 0}), {});
    slv.assertFormula(d);
    std::cout << logic << " -> " << slv.checkSat() << "\n";
  }
  catch (const std::exception& e)
  {
    std::cout << logic << " -> EXCEPTION: " << e.what() << "\n";
  }
}

int main()
{
  tryLogic("ALL");
  tryLogic("QF_PLAN");
  tryLogic("QF_UF");
  tryLogic("QF_PLANLIA");
  return 0;
}
