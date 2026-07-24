#include <cvc5/cvc5.h>

#include <iostream>

using namespace cvc5;

int main()
{
  TermManager tm;
  Solver slv(tm);
  slv.setOption("produce-models", "true");
  slv.setLogic("QF_PLANLIA");

  // does(0,0) : action 0 at timestep 0
  Op doesOp = tm.mkOp(Kind::PLAN_DOES, {0, 0});
  Term does00 = tm.mkTerm(doesOp, {});

  // fluent(7,1) : boolean fluent 7 at timestep 1
  Op flOp = tm.mkOp(Kind::PLAN_FLUENT, {7, 1});
  Term fl71 = tm.mkTerm(flOp, {});

  // aux(3,0)
  Op auxOp = tm.mkOp(Kind::PLAN_AUX, {3, 0});
  Term aux30 = tm.mkTerm(auxOp, {});

  // an Int-sorted fluent via the dedicated entry point
  Term fuel = tm.mkPlanFluent(42, 2, tm.getIntegerSort());

  std::cout << "does00      = " << does00 << "  : " << does00.getSort() << "\n";
  std::cout << "fl71        = " << fl71 << "  : " << fl71.getSort() << "\n";
  std::cout << "aux30       = " << aux30 << "  : " << aux30.getSort() << "\n";
  std::cout << "fuel        = " << fuel << "  : " << fuel.getSort() << "\n";
  std::cout << "kind        = " << does00.getKind() << "\n";
  std::cout << "op indices  = " << doesOp.getNumIndices() << " -> "
            << doesOp[0] << ", " << doesOp[1] << "\n";

  // hash-consing: same (index,timestep,sort) => same term
  Term does00bis = tm.mkTerm(tm.mkOp(Kind::PLAN_DOES, {0, 0}), {});
  std::cout << "hashconsed  = " << (does00 == does00bis ? "yes" : "no") << "\n";

  // assert:  does00  and  (=> does00 fl71)  and  (not aux30)
  slv.assertFormula(does00);
  slv.assertFormula(tm.mkTerm(Kind::IMPLIES, {does00, fl71}));
  slv.assertFormula(tm.mkTerm(Kind::NOT, {aux30}));
  slv.assertFormula(tm.mkTerm(Kind::GT, {fuel, tm.mkInteger(3)}));

  Result r = slv.checkSat();
  std::cout << "result      = " << r << "\n";
  if (r.isSat())
  {
    std::cout << "model does00 = " << slv.getValue(does00) << "\n";
    std::cout << "model fl71   = " << slv.getValue(fl71) << "\n";
    std::cout << "model aux30  = " << slv.getValue(aux30) << "\n";
    std::cout << "model fuel   = " << slv.getValue(fuel) << "\n";
  }
  return 0;
}
