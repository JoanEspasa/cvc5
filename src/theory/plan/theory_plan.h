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
 * At this stage the theory is deliberately inert: it accepts every fact and
 * never propagates, conflicts or lemmas. Its only job is to exist before the
 * encoder is retyped against cvc5, so that atom identity is decided once. See
 * docs/cvc5/migration-plan.md (M3) in the RantanPlan repository.
 */

#include "cvc5_private.h"

#ifndef CVC5__THEORY__PLAN__THEORY_PLAN_H
#define CVC5__THEORY__PLAN__THEORY_PLAN_H

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
   * Fact delivery.
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
}; /* class TheoryPlan */

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal

#endif /* CVC5__THEORY__PLAN__THEORY_PLAN_H */
