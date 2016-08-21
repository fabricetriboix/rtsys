/* Copyright (c) 2014-2016  Fabrice Triboix
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rthsm.h"
#include "rtplf.h"



/*-------------------------------+
 | Private function declarations |
 +-------------------------------*/


/** Lookup a state from its id
 *
 * @param hsm [in] The state machine to query
 * @param id  [in] State id to lookup
 *
 * @return A pointer to the found state structure, or NULL if no state with
 *         this id has been found
 */
static RTHsmState* rthsmLookupStateFromId(const RTHsm* hsm, uint8_t id);


/* Transition to the deepest child state
 *
 * This function will transition to the childmost sub-state of the given
 * `state`, executing entry actions as appropriate.
 *
 * `hsm->current` will be set to the childmost sub-state.
 *
 * @param hsm   [in,out] The state machine to work on
 * @param state [in]     The state to start from
 */
static void rthsmTraverseToChildmostState(RTHsm* hsm, RTHsmState* state);


/* Get the best transition possible for the given event
 *
 * The best transition starts from the current state or one of its parents, is
 * triggered by the given event id, and the guard condition (if there is one),
 * allows the transition.
 *
 * If a transition triggered by the given event id is found, but the guard
 * condition denies the transition, the `guardResult` argument will be set to
 * the value returned by the guard condition and this function returns NULL.
 *
 * If no transition is found, this function returns NULL and `guardResult` is
 * set to 0.
 *
 * @param hsm         [in]  The state machine to query
 * @param event       [in]  The received event
 * @param guardResult [out] The value returned by the guard condition that
 *                          failed, if applicable. Must not be NULL.
 *
 * @return A pointer to the found transition, or NULL if no transition found
 */
static RTHsmTransition* rthsmGetBestTransition(const RTHsm* hsm,
        const RTHsmEvent* event, uint8_t* guardResult);


/* Perform a transition of the HSM's current state
 *
 * @param hsm        [in,out] The state machine to action
 * @param transition [in]     The transition to execute
 * @param event      [in]     The event that triggered the transition
 */
static void rthsmDoSelfTransition(RTHsm* hsm,
        const RTHsmTransition* transition, const RTHsmEvent* event);


/* Perform a transition which is not a self-transition
 *
 * A transition that is not a self-transition has a destination state that is
 * different from the originating state.
 *
 * @param hsm        [in,out] The state machine to action
 * @param transition [in]     The transition to execute
 * @param event      [in]     The event that triggered the transition
 */
static void rthsmDoTransition(RTHsm* hsm,
        const RTHsmTransition* transition, const RTHsmEvent* event);



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


void RTHsmInit(RTHsm* hsm, RTHsmState* states, uint8_t statesSize,
        RTFifo* eventQueue)
{
    uint8_t i;

    RTASSERT(hsm != NULL);
    RTASSERT(states != NULL);
    RTASSERT(statesSize > 0);
    RTASSERT(eventQueue != NULL);

    hsm->states = states;
    hsm->statesSize = statesSize;
    hsm->global = NULL;
    hsm->current = NULL;
    hsm->eventQueue = eventQueue;

    /* Cache state pointers from ids, & check there is only one global state */
    for (i = 0; i < hsm->statesSize; i++) {
        uint8_t j;
        RTHsmState* state = &(hsm->states[i]);

        /* Check that this state id is unique */
        for (j = i + 1; j < hsm->statesSize; j++) {
            RTASSERT(hsm->states[j].id != state->id);
        }

        /* Populate `parent` and check there is only one global state */
        if (state->parentId == RTHSM_NULL_STATE_ID) {
            /* `state` is the global state => Check there are not more than 1 */
            RTASSERT(hsm->global == NULL);
            state->parent = NULL;
            hsm->global = state;
        } else {
            state->parent = rthsmLookupStateFromId(hsm, state->parentId);
            RTASSERT(state->parent != NULL);
        }

        /* Populate `initial` */
        if (state->initialId == RTHSM_NULL_STATE_ID) {
            state->initial = NULL;
        } else {
            state->initial = rthsmLookupStateFromId(hsm, state->initialId);
            RTASSERT(state->initial != NULL);
        }
    }

    /* Check there is a global state */
    RTASSERT(hsm->global != NULL);

    /* Check the global state has an initial sub-state */
    RTASSERT(hsm->global->initial != NULL);

    /* Check the global state has no transition */
    RTASSERT(hsm->global->transitionsSize == 0);

    /* Check state hierarchy */
    for (i = 0; i < hsm->statesSize; i++) {
        RTHsmState* state = &(hsm->states[i]);

        /* Check that each `initial` state as the right `parent` */
        if (state->initial != NULL) {
            RTASSERT(state->initial->parent == state);
        }

        /* Check that each state ultimately belongs to the global state
         *
         * NB: No need to do this check for the global state itself!
         */
        if (state != hsm->global) {
            uint8_t j;
            RTBool belongsToGlobalState = RTFalse;
            RTHsmState* iter = state;

            for (j = 0; j < RTHSM_MAX_NESTED_STATES; j++) {
                if (iter->parent == hsm->global) {
                    belongsToGlobalState = RTTrue;
                    break;
                }
                iter = iter->parent;
            }
            RTASSERT(belongsToGlobalState);
        }
    }

    /* Check and populate transition structures */
    for (i = 0; i < hsm->statesSize; i++) {
        uint8_t j;
        RTHsmState* iter = &(hsm->states[i]);

        for (j = 0; j < iter->transitionsSize; j++) {
            RTHsmState* state;
            RTHsmTransition* transition = &(iter->transitions[j]);
            RTASSERT(transition->toStateId != RTHSM_NULL_STATE_ID);

            state = rthsmLookupStateFromId(hsm, transition->toStateId);
            RTASSERT(state != NULL);
            RTASSERT(state != hsm->global);

            transition->toState = state;
        }
    }
}


RTBool RTHsmPushEvent(RTHsm* hsm, const RTHsmEvent* event)
{
    return RTFifoPush(hsm->eventQueue, event, sizeof(*event));
}


RTHsmResult RTHsmStep(RTHsm* hsm, uint8_t* guardResult)
{
    RTHsmResult result;
    RTHsmEvent event;

    RTASSERT(hsm != NULL);

    if (hsm->current == NULL) {
        /* This is the first time `RTHsmStep()` is called */
        rthsmTraverseToChildmostState(hsm, hsm->global);
        result = RTHSM_STEP_RESULT_OK;

    } else if (hsm->current->flags & RTHSM_STATE_FLAG_FINAL) {
        /* This state machine is now terminated */
        result = RTHSM_STEP_RESULT_TERMINATED;

    } else if (!RTFifoPop(hsm->eventQueue, &event, sizeof(event))) {
        result = RTHSM_STEP_RESULT_EMPTY;

    } else {
        uint8_t gresult;
        RTHsmTransition* transition = rthsmGetBestTransition(hsm,
                &event, &gresult);

        if (transition == NULL) {
            if (gresult != 0) {
                result = RTHSM_STEP_RESULT_GUARD;
                if (guardResult != NULL) {
                    *guardResult = gresult;
                }
            } else {
                result = RTHSM_STEP_RESULT_DISCARDED;
            }
        } else {
            if (transition->toState == hsm->current) {
                rthsmDoSelfTransition(hsm, transition, &event);
            } else {
                rthsmDoTransition(hsm, transition, &event);
            }
            result = RTHSM_STEP_RESULT_OK;
        }
    }
    return result;
}



/*----------------------------------+
 | Private function implementations |
 +----------------------------------*/


static RTHsmState* rthsmLookupStateFromId(const RTHsm* hsm, uint8_t id)
{
    RTHsmState* state = NULL;
    uint8_t i;

    RTASSERT(hsm != NULL);

    for (i = 0; (i < hsm->statesSize) && (state == NULL); i++) {
        if (hsm->states[i].id == id) {
            state = &(hsm->states[i]);
        }
    }
    return state;
}


static void rthsmTraverseToChildmostState(RTHsm* hsm, RTHsmState* state)
{
    RTHsmState* substate;

    RTASSERT(hsm != NULL);
    RTASSERT(state != NULL);

    hsm->current = state;
    do {
        substate = hsm->current->initial;
        if (substate != NULL) {
            hsm->current = substate;
            if (substate->entryAction != NULL) {
                substate->entryAction(substate->cookie);
            }
        }
    } while (substate != NULL);
}


static RTHsmTransition* rthsmGetBestTransition(const RTHsm* hsm,
        const RTHsmEvent* event, uint8_t* guardResult)
{
    RTHsmTransition* transition = NULL;
    RTHsmState* state;
    uint8_t i;

    RTASSERT(hsm != NULL);
    RTASSERT(hsm->current != NULL);
    RTASSERT(event != NULL);
    RTASSERT(guardResult != NULL);

    state = hsm->current;

    *guardResult = 0; /* Assume no transition found */

    /* Try each state in the hierarchy in turn, starting from the innermost one
     * up to the global state.
     */
    for (i = 0; (state != hsm->global) && (transition == NULL); i++) {
        uint8_t j;
        RTASSERT(i < RTHSM_MAX_NESTED_STATES);

        for (j = 0; (j < state->transitionsSize) && (transition == NULL); j++) {
            RTHsmTransition* iter = &(state->transitions[j]);

            if (iter->eventId == event->id) {
                /* We found a transition triggered by this event id
                 *  => Check if the guard condition let us do it
                 */
                if (iter->guard != NULL) {
                    *guardResult = iter->guard(event, iter->cookie);
                    if (0 == *guardResult) {
                        transition = iter; /* Guard condition says "go" */
                    }
                } else {
                    transition = iter; /* No guard condition */
                }
            } /* If this transition is triggered by the event */
        } /* For each transition originating from this state */

        state = state->parent;
    } /* For each parent state */

    return transition;
}


static void rthsmDoSelfTransition(RTHsm* hsm,
        const RTHsmTransition* transition, const RTHsmEvent* event)
{
    RTASSERT(hsm != NULL);
    RTASSERT(hsm->current != NULL);
    RTASSERT(transition != NULL);
    RTASSERT(event != NULL);

    /* NB: For transition that are not internal, we have to execute the exit and
     * entry actions of the state.
     */
    if (    !(transition->flags & RTHSM_TRANSITION_FLAG_INTERNAL)
         && (hsm->current->exitAction != NULL) ) {
        hsm->current->exitAction(hsm->current->cookie);
    }

    if (transition->action != NULL) {
        transition->action(event, transition->cookie);
    }

    if (    !(transition->flags & RTHSM_TRANSITION_FLAG_INTERNAL)
         && (hsm->current->entryAction != NULL) ) {
        hsm->current->entryAction(hsm->current->cookie);
    }
}


static void rthsmDoTransition(RTHsm* hsm,
        const RTHsmTransition* transition, const RTHsmEvent* event)
{
    RTHsmState* dstParents[RTHSM_MAX_NESTED_STATES + 1];
    int8_t      dstParentsCount;
    int8_t      i;
    RTHsmState* state;
    RTHsmState* commonParent;
    int8_t      commonParentIndex;

    /* Check arguments */
    RTASSERT(hsm != NULL);
    RTASSERT(transition != NULL);
    RTASSERT(transition->toState != NULL);
    RTASSERT(event != NULL);

    /* Build the list of parents of the destination state, including itself */
    dstParents[0] = transition->toState;
    dstParentsCount = 1;
    while (dstParents[dstParentsCount - 1] != hsm->global) {
        RTASSERT(dstParentsCount <= RTHSM_MAX_NESTED_STATES);

        dstParents[dstParentsCount] = dstParents[dstParentsCount - 1]->parent;
        dstParentsCount++;
    }

    /* Find the nearest common parent between the originating state and the
     * destination state (in last resort, the global state is the parent of all
     * states).
     * NB: We do need to test the originating or the destination states as one
     * might be nested into the other.
     */
    commonParentIndex = -1;
    commonParent = NULL;
    for (i = 0; (i < dstParentsCount) && (commonParent == NULL); i++) {
        RTHsmState* lastState = NULL;
        for (   state = hsm->current;
                (state != hsm->global) && (commonParent == NULL);
                state = state->parent) {
            lastState = state;
            if (state == dstParents[i]) {
                commonParentIndex = i;
                commonParent = dstParents[i];
            }
        }

        if ((commonParent == NULL) && (lastState != NULL)) {
            if (lastState->parent == dstParents[i]) {
                commonParentIndex = i;
                commonParent = dstParents[i];
            }
        }
    }
    RTASSERT(commonParentIndex >= 0);
    RTASSERT(commonParent != NULL);

    /* Execute exit actions from the childmost originating state to the common
     * parent.
     * Note: Do not execute the exit action of the common parent, because this
     * state is not exited.
     */
    for (state = hsm->current; state != commonParent; state = state->parent) {
        if (state->exitAction != NULL) {
            state->exitAction(state->cookie);
        }
    }

    /* Execute transition action */
    if (transition->action != NULL) {
        transition->action(event, transition->cookie);
    }

    /* Execute entry actions from the common parent to childmost destination
     * state.
     * Note: Do not execute the entry action of the common parent, because this
     * state is not entered.
     */
    for (i = commonParentIndex - 1; i >= 0; i--) {
        state = dstParents[i];
        if (state->entryAction != NULL) {
            state->entryAction(state->cookie);
        }
    }

    /* If the destination state has children states, we must go deeper into the
     * nesting of states until we reach a state without nested states.
     */
    rthsmTraverseToChildmostState(hsm, transition->toState);
}
