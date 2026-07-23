/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Rewriter for the theory of planning.
 */

#include "cvc5_private.h"

#ifndef CVC5__THEORY__PLAN__THEORY_PLAN_REWRITER_H
#define CVC5__THEORY__PLAN__THEORY_PLAN_REWRITER_H

#include "theory/theory_rewriter.h"

namespace cvc5::internal {
namespace theory {
namespace plan {

/**
 * The planning rewriter, deliberately a no-op.
 *
 * A planning atom is an opaque proposition identified by its operator index;
 * there is no algebraic identity to exploit and nothing to normalise. Returning
 * REWRITE_DONE unchanged is also what keeps the atoms intact: it is the
 * rewriter's job to stop a kind whose entire content is a constant operator
 * from being folded away before it reaches CNF conversion.
 */
class TheoryPlanRewriter : public TheoryRewriter
{
 public:
  TheoryPlanRewriter(NodeManager* nm);

  RewriteResponse postRewrite(TNode node) override
  {
    return RewriteResponse(REWRITE_DONE, node);
  }

  RewriteResponse preRewrite(TNode node) override
  {
    return RewriteResponse(REWRITE_DONE, node);
  }
}; /* class TheoryPlanRewriter */

}  // namespace plan
}  // namespace theory
}  // namespace cvc5::internal

#endif /* CVC5__THEORY__PLAN__THEORY_PLAN_REWRITER_H */
