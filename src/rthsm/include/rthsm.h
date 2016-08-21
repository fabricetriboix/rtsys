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

/** Hierarchical state machine
 *
 * @defgroup rthsm Hierarchical state machine
 * @addtogroup rthsm
 * @{
 *
 * This module implements hierarchical state machines based on a subset of UML
 * 2.0 state machine diagrams. In particular, it does not implement any of the
 * pseudo-states (the inital and final pseudo-state are supported in a derived
 * form).
 *
 * In addition, it does not implement `do` actions. Such actions are triggered
 * at every step of the state machine if the current state has one. This is very
 * inefficient and is bad design, especially for an embedded/real-time
 * application. You can implement a similar action using an internal transition
 * with an action. The action will then be executed only when the triggering
 * event has been received, which is much more efficient.
 *
 * Another constraint is that all transitions must be triggered by an event.
 * Transitions triggered by a guard condition becoming true are inefficient
 * because the conditions must be evaluated every time the state machine is
 * stepped, until the transition is actually performed. Unconditional
 * transitions are uncommon and can very easily be implemented using an event.
 *
 * A state machine has the following constraints:
 *  - State ids must be unique
 *  - There must be one and only one global state (with the `parentId` set to
 *    `RTHSM_NULL_STATE_ID`)
 *  - The global state must have an initial sub-state
 *  - The global state must not have any transitions originating from it
 *  - For each state, `parentId` and `initialId` must refer to existing states
 *    within this state machine
 *  - If a state A has an initial sub-state B, that sub-state B must have the
 *    state A for parent
 *  - All states must ultimately belong to the global state
 *  - The level of state nesting must not exceed `RTHSM_MAX_NESTED_STATES` and
 *    does not include the global state
 *    For example, if `RTHSM_MAX_NESTED_STATES` is set to 3, the following
 *    nesting would be the maximum allowed:
 *      global -> state A -> state B -> state C
 *  - All transitions must have a valid destination state: `toStateId` must not
 *    be `RTHSM_NULL_STATE_ID`, must not be the global state, and must point to
 *    an existing state within this state machine
 */

/* TODO: time-triggered scheduler */

#ifndef RTHSM_h_
#define RTHSM_h_

#include "rtplf.h"
#include "rtfifo.h"



/*--------+
 | Macros |
 +--------*/


/** Number of parameters in an event */
#define RTHSM_EV_MAX_PARAMS 2


/** Null state id
 *
 * Do not use this id for any of your states!
 */
#define RTHSM_NULL_STATE_ID 0u


/** State flag indicating that this state is final
 *
 * The state machine engine will stop when it transitions to a state with this
 * flag. If this state has an entry action, it will be executed before the state
 * machine is stopped.
 */
#define RTHSM_STATE_FLAG_FINAL 0x01u


/** Maximum level of nested states */
#define RTHSM_MAX_NESTED_STATES 3


/** Transition flag indicating that this is an internal transition
 *
 * This flag is valid only when the destination state is the same as the source
 * state. It indicates that the entry and exit actions of that state should not
 * be performed.
 * If the destination state is the same as the source state and this flag is not
 * present, the entry and exit actions (if applicable) of that state will be
 * called.
 *
 * If the destination state is not the same as the source state, this flag is
 * ignored.
 */
#define RTHSM_TRANSITION_FLAG_INTERNAL 0x01u



/*-------+
 | Types |
 +-------*/


/** All possible results from a state machine step */
typedef enum
{
    RTHSM_STEP_RESULT_OK,        /**< Event de-queued and transition actioned */
    RTHSM_STEP_RESULT_EMPTY,     /**< Event queue is empty */
    RTHSM_STEP_RESULT_DISCARDED, /**< Event de-queued, but does nothing */
    RTHSM_STEP_RESULT_GUARD,     /**< Guard condition failed */
    RTHSM_STEP_RESULT_TERMINATED /**< State machine is now terminated */
} RTHsmResult;


/** Event structure */
typedef struct
{
    uint8_t  id;                          /**< Event id */
    uint32_t params[RTHSM_EV_MAX_PARAMS]; /**< Event parameters */
} RTHsmEvent;


/** Transition guard condition
 *
 * Prototype of a function that is called when a transition is about to be
 * actioned. The guard condition can deny the transition by return a non-zero
 * value.
 *
 * *VERY IMPORTANT*: A transition guard should have no side-effect. If it had a
 * side-effect and denied the transition, the state of the system would have
 * changed but the state machine would remain the same, thus defeating the
 * purpose of a state machine in the first place. If a side-effect is required
 * as part of the transition, please use the transition action, see
 * `RTHsmTransitionAction`. If a side-effect is required as part of the
 * evaluation of the guard condition, your state machine is not designed
 * properly and probably needs at least one more state.
 *
 * If it returns > 0:
 *  - The guard condition is considered failed
 *  - The transition will not be actioned
 *  - No entry/exit action of states involved in this transition will be called
 *  - The transition action will not be called
 *  - The return value will be populated when you call `RTHsmStep()`
 *
 * If it returns 0:
 *  - The guard condition is considered to have succeeded
 *  - The transition will be actioned
 *  - All relevant exit/entry actions of states involved with this transition
 *    will be called in the right order
 *  - The transition action will be called at the right time
 *
 * The arguments are:
 *  - `event` is the event that triggered the tentative transition
 *  - `cookie` is the value of the `RTHsmTransition.cookie` field
 */
typedef uint8_t (*RTHsmTransitionGuard)(const RTHsmEvent* event, void* cookie);


/** Transition action
 *
 * Prototype of a function that is called when the transition is actioned.
 *
 * The arguments are:
 *  - `event` is the event that triggered the transition
 *  - `cookie` is the value of the `RTHsmTransition.cookie` field
 */
typedef void (*RTHsmTransitionAction)(const RTHsmEvent* event, void* cookie);


/* Forward declaration; needed for `RTHsmTransition` */
struct RTHsmState;

/** Structure describing a transition */
typedef struct
{
    /** Id of the destination state */
    uint8_t toStateId;

    /** Id of the triggering event */
    uint8_t eventId;

    /** Transition flags, see `RTHSM_TRANSITION_FLAG_*` */
    uint8_t flags;

    /** Guard condition
     *
     * Set to NULL if there is no guard condition. In that case, the engine will
     * act as if there was a guard condition that always succeeds.
     */
    RTHsmTransitionGuard guard;

    /** Transition action, or NULL if no action */
    RTHsmTransitionAction action;

    /** Cookie for transition action */
    void* cookie;

    struct RTHsmState* toState; /**< Private stuff, just set to NULL */
} RTHsmTransition;


/** State action, could be either entry or exit action
 *
 * Prototype of a function that is called when a state is entered or exited.
 *
 * The `cookie argument is the value of the `RTHsmState.cookie` field.
 */
typedef void (*RTHsmStateAction)(void* cookie);


/** Structure describing a state */
typedef struct RTHsmState
{
    /** State id, must be unique and not `RTHSM_NULL_STATE_ID` */
    uint8_t id;

    /** State flags, see `RTHSM_STATE_FLAG_*` */
    uint8_t flags;

    /** Parent state; `RTHSM_NULL_STATE_ID` if this is the global state */
    uint8_t parentId;

    /** Initial nested state, or `RTHSM_NULL_STATE_ID` if there is no initial nested state */
    uint8_t initialId;

    /** Entry action for this state, NULL if no entry action */
    RTHsmStateAction entryAction;

    /** Exit action for this state, NULL if no exit action */
    RTHsmStateAction exitAction;

    /** Cookie for entry and exit actions */
    void* cookie;

    /** Transitions originating from this state
     *
     * NB: These include self-transitions.
     */
    RTHsmTransition* transitions;

    /** Size of the above `transitions` array */
    uint8_t transitionsSize;

    struct RTHsmState* parent;  /**< Private stuff, just set to NULL */
    struct RTHsmState* initial; /**< Private stuff, just set to NULL */
} RTHsmState;


/** Structure describing a state machine
 *
 * This structure should not be populated directly; use `RTHsmInit()` to
 * initialise this structure.
 */
typedef struct
{
    RTHsmState* states;     /**< Array of states */
    uint8_t     statesSize; /**< Size of `states` array */
    RTHsmState* global;     /**< The global state for this state machine */
    RTHsmState* current;    /**< Current state */
    RTFifo*     eventQueue; /**< Event queue */
} RTHsm;



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Initialise a state machine
 *
 * This function must be called on a state machine structure before any other
 * function.
 *
 * If any of the constraint detailed above is broken, this function will panic.
 *
 * Please note there is always an implied transition: the very first transition
 * is when entering the initial sub-state of the global state. This implied
 * transition is unconditional, has no action, and always happen the first time
 * the state machine is stepped.
 *
 * @param hsm        [out]    The HSM structure to initialise
 * @param states     [in,out] Array of states for this state machine. This
 *                            array (and any sub-array such as transitions)
 *                            must be statically allocated. The ownership of
 *                            this array is transferred to this module, do not
 *                            touch any part of the array once `RTHsmInit()` is
 *                            called.
 * @param statesSize [in,out] Size of the above array
 * @param eventQueue [in,out] Event queue to use; the ownership is transferred
 *                            to this module, do not touch the FIFO once
 *                            `RTHsmInit()` is called.
 */
void RTHsmInit(RTHsm* hsm, RTHsmState* states, uint8_t statesSize,
        RTFifo* eventQueue);


/** Push an event to a state machine
 *
 * This is just a utility function that encapsulates a call to push the event
 * onto the state machine queue.
 *
 * @param hsm   [in,out] The HSM where to push the event
 * @param event [in]     The event to push; a copy of the `event` will be made,
 *                       so the ownership of the `event` remains with you.
 *
 * @return `RTTrue` if success, `RTFalse` if event queue is full.
 */
RTBool RTHsmPushEvent(RTHsm* hsm, const RTHsmEvent* event);


/** Execute one step of the state machine
 *
 * Please note that the very first time this function is called, it will
 * normally return `RTHSM_RESULT_OK` even if the event queue is empty.
 *
 * @param hsm         [in,out] The state machine to step
 * @param guardResult [out]    The value returned by the guard condition that
 *                             failed. May be NULL if you are not interested in
 *                             this information. Only set when `RTHsmStep()`
 *                             returns `RTHSM_RESULT_GUARD`.
 *
 * @return
 *  - `RTHSM_STEP_RESULT_OK`: An event has been de-queued, processed and a
 *     transition occurred.
 *  - `RTHSM_STEP_RESULT_EMPTY`: Event queue is empty, no action taken.
 *  - `RTHSM_STEP_RESULT_DISCARDED`: There was an event in the queue, but it
 *     didn't trigger any transition and has been discarded.
 *  - `RTHSM_STEP_RESULT_GUARD`: A guard condition prevented the transition from
 *    occurring; if the `guardResult` argument is not NULL, it is set to the
 *    value returned by the guard condition that failed.
 *  - `RTHSM_STEP_RESULT_TERMINATED`: This state machine is terminated.
 */
RTHsmResult RTHsmStep(RTHsm* hsm, uint8_t* guardResult);


/** Reset a state machine
 *
 * The state machine will go back to a state identical to what it was after the
 * call to `RTHsmInit()` and before any call to `RTHsmStep()`.
 *
 * @param hsm [in,out] The state machine to reset
 */
void RTHsmReset(RTHsm* hsm);


#endif
/* @} */
