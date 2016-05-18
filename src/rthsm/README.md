rthsm
=====

rthsm is an implementation of a hierarchical state machine engine
based on UML 2.0 state machine diagrams.

What is implemented from UML 2.0:
 - States
 - Transitions between states
 - Nested states (composite states)
 - Transition triggers (events)
 - Transition guard conditions
 - Self transitions
 - Internal transitions
 - Deep transitions (transitions that cross state boundaries and go to
   or from nested states)
 - Actions for:
   * State entry action
   * State exit action
   * Transition action
 - The following pseudo-states:
   * A special form of initial pseudo-state

Constraints:
 - Transitions must be triggered by an event

What is NOT implemented:
 - Any pseudo-state except for the initial pseudo-state (albeit in a
   derived form)
   Rationale: Pseudo-states are a source of confusion and are
   generally useless
 - 'Do' behaviour
   Rationale: This would imply constant polling
 - Transitions without a trigger (event)
   Rationale: Avoid constant/regular polling to know if a transition
   can be actioned or not
 - Orthogonal regions
   Rationale: (a) I have the feeling it might break determinism and
   (b) I can't think of a use case where a state machine might need to
   fork at some point to join later (and parallel state machines can
   be defined using rthsm anyway)

