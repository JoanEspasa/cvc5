/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The theory of planning.
 *
 * Owns the PLAN_DOES / PLAN_FLUENT / PLAN_AUX entities of a
 * planning-as-satisfiability encoding, so that planning-specific reasoning --
 * exists-step cycle detection, forall-step mutex propagation, lazy frame
 * axioms -- can run inside the solver rather than as an external propagator.
 *
 * The theory reasons only when an interference oracle has been registered
 * (Solver::setPlanInterferenceOracle). Without one it observes nothing and
 * costs nothing, which is what lets an encoding that fixes its parallelism a
 * priori share these atoms without paying for machinery it does not want.
 *
 * Which parallel-step semantics is enforced is chosen by the embedder
 * (Solver::configurePlanTheory), because both are decided from the same
 * interference relation and differ only in what they demand of it:
 *
 * - ExistsStep: the actions must be executable in *some* order, which holds
 *   exactly when the relation restricted to them is acyclic. A cycle means
 *   every candidate order needs one action both before and after another, so
 *   the cycle's literals are the conflict.
 * - ForallStep: the actions must be executable in *any* order, so a single
 *   interfering pair, in either direction, is already the conflict.
 */

#include "cvc5_private.h"

#ifndef CVC5__THEORY__PLAN__THEORY_PLAN_H
#define CVC5__THEORY__PLAN__THEORY_PLAN_H

#include <cstdint>
#include <vector>

#include "context/cdlist.h"
#include "expr/node.h"
#include "smt/env_obj.h"
#include "theory/plan/theory_plan_rewriter.h"
#include "theory/theory.h"
#include "theory/theory_inference_manager.h"
#include "theory/theory_state.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

class TheoryPlan : public Theory
{
 public:
  TheoryPlan(Env& env, OutputChannel& out, Valuation valuation);
  ~TheoryPlan() override;

  //--------------------------------- initialization
  /** get the official theory rewriter of this theory */
  TheoryRewriter* getTheoryRewriter() override;
  /**
   * Returns nullptr: this theory produces no proofs. That is the normal case
   * for a theory without a proof calculus (compare theory_ff.cpp,
   * theory_bags.cpp, theory_sep.cpp); its lemmas are wrapped as
   * TrustId::THEORY_LEMMA. Do not enable --produce-proofs against it.
   */
  ProofRuleChecker* getProofChecker() override;
  /**
   * Returns false: planning atoms are opaque nullary predicates that are never
   * congruent and never equated, so there is nothing for an equality engine to
   * do. This choice is what makes preNotifyFact the delivery point -- see
   * below.
   */
  bool needsEqualityEngine(EeSetupInfo& esi) override;
  //--------------------------------- end initialization

  /**
   * Fact delivery, and the theory's reasoning hook.
   *
   * Theory::check drains the fact queue through this method, one literal at a
   * time in assignment order, at every check round -- including
   * EFFORT_STANDARD, i.e. at each propagation fixpoint during the descent.
   * That is what makes it the place to reason *incrementally*, as opposed to
   * EFFORT_FULL, which runs only once the SAT solver already holds a complete
   * assignment and would amount to generating a candidate plan and testing it.
   *
   * Returns true, meaning "handled; do not forward to the equality engine".
   * A theory with no equality engine *must* return true here: returning false
   * makes Theory::check fall through to notifyFact, whose
   * Assert(d_equalityEngine != nullptr) would then fire.
   */
  bool preNotifyFact(
      TNode atom, bool pol, TNode fact, bool isPrereg, bool isInternal) override;

  /**
   * Put the truth value of every asserted planning atom into the model.
   *
   * Not optional, despite the theory being otherwise inert. A planning atom has
   * *no children* -- its whole content is the operator index -- so
   * TheoryModel::getModelValue cannot evaluate it from subterms, and with no
   * equality engine there is nothing else to supply a value. Without this the
   * model simply has no entry for a PLAN_* atom, which is fatal downstream:
   * reading those values back out of the model is precisely how a plan is
   * extracted.
   */
  bool collectModelValues(TheoryModel* m,
                          const std::set<Node>& termSet) override;

  std::string identify() const override { return "THEORY_PLAN"; }

 private:
  /** The theory rewriter for this theory. */
  TheoryPlanRewriter d_rewriter;
  /** The state of the theory. */
  TheoryState d_state;
  /** The inference manager. */
  TheoryInferenceManager d_im;

  /**
   * One action asserted to execute: which action, at which timestep, and the
   * atom itself.
   *
   * The atom is kept rather than rebuilt so a conflict is expressed in exactly
   * the literals the SAT solver asserted, with no reliance on reconstructing an
   * identical term.
   */
  struct ActiveAction
  {
    uint32_t d_action;
    uint32_t d_timestep;
    Node d_atom;
  };

  /**
   * The actions asserted true so far.
   *
   * Held on the SAT context, so the solver's own backtracking maintains it:
   * what an external propagator needs an explicit trail, decision-level stack
   * and push/pop bookkeeping for is here just the choice of context. Populated
   * only while an interference oracle is registered.
   */
  context::CDList<ActiveAction> d_activeActions;

  /**
   * Find a cycle in the interference relation restricted to `actions`.
   *
   * Plain DFS over a graph whose edges are discovered by asking the oracle,
   * rather than materialised in advance -- the point of answering interference
   * lazily. Returns true and fills `cycle` with the actions forming it.
   *
   * The relation is directional, so `interferes(x, y)` is asked for every
   * ordered pair; a mutually-interfering pair is a two-element cycle.
   */
  bool findInterferenceCycle(const std::vector<uint32_t>& actions,
                             cvc5::PlanInterferenceOracle& oracle,
                             std::vector<uint32_t>& cycle) const;

  /**
   * Find an action in `actions` that interferes with `action` in either
   * direction, for forall-step semantics.
   *
   * Only pairs involving `action` are examined, which is complete because any
   * other pair was already checked when the later of its two members was
   * asserted.
   */
  bool findInterferingPartner(uint32_t action,
                              const std::vector<uint32_t>& actions,
                              cvc5::PlanInterferenceOracle& oracle,
                              uint32_t& other) const;
}; /* class TheoryPlan */

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal

#endif /* CVC5__THEORY__PLAN__THEORY_PLAN_H */
