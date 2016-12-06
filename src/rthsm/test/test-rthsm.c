/* Copyright (c) 2015-2016  Fabrice Triboix
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

/* Please read the `readme.txt` file in this directory, it has a UML2.0 diagram
 * detailing the state machine used in this unit test.
 */

#include "rthsm.h"
#include "rttest.h"
#include "rtplf.h"
#include "rtfifo.h"

#if (RTHSM_MAX_NESTED_STATES != 3)
#error "RTHSM_MAX_NESTED_STATES must be set to 3"
#endif


/* State ids */
#define STATE_ID_GLOBAL 1
#define STATE_ID_STARTING 2
#define STATE_ID_DEVICE_ON 3
#define STATE_ID_FINISHED 4
#define STATE_ID_ACTIVE 5
#define STATE_ID_READING 6
#define STATE_ID_PROCESSING 7
#define STATE_ID_SAVING 8
#define STATE_ID_ERROR 9
#define STATE_ID_MALFUNCTION 10


/* Event ids */
#define EV_DATA 1
#define EV_ACQUIRED 2
#define EV_PROCESSING 3
#define EV_PROCESSED 4
#define EV_SAVED 5
#define EV_RECOVER 6
#define EV_ERROR 7
#define EV_DEAD 8
#define EV_NEXT 99


/* State machine and its variables */
static RTHsm gHsm;
static int8_t gIteration = 0;
static int8_t gProcessCount = 99;
static uint8_t gMalfunctionGuard = 123u;

/* State machine message queue */
static RTHsmEvent gEventsBuffer[8];
static RTFifo gEventQueue = RT_FIFO_INIT(gEventsBuffer);


/* Definition of transitions originating from the "Starting" state */

static uint8_t rthsmTestStartingToDeviceOnGuard(const RTHsmEvent* event,
        void* cookie)
{
    uint8_t ret = 1u;

    (void)event; /* unused argument */
    (void)cookie; /* unused argument */

    if (gIteration < 5) {
        ret = 0;
    }
    return ret;
}

static uint8_t rthsmTestStartingToFinishedGuard(const RTHsmEvent* event,
        void* cookie)
{
    uint8_t retval = 1u;

    (void)event; /* unused argument */
    (void)cookie; /* unused argument */

    if (gIteration >= 5) {
        retval = 0;
    }
    return retval;
}

static RTHsmTransition gStartingTransitions[] =
{
    {
        STATE_ID_DEVICE_ON,               /* toStateId */
        EV_NEXT,                          /* eventId */
        0,                                /* flags */
        rthsmTestStartingToDeviceOnGuard, /* guard */
        NULL,                             /* action */
        NULL,                             /* cookie */
        NULL                              /* private: toState */
    },
    {
        STATE_ID_FINISHED,                /* toStateId */
        EV_NEXT,                          /* eventId */
        0,                                /* flags */
        rthsmTestStartingToFinishedGuard, /* guard */
        NULL,                             /* action */
        NULL,                             /* cookie */
        NULL                              /* private: toState */
    }
};


/* Definition of transitions originating from the "DeviceOn" state */

static RTHsmTransition gDeviceOnTransitions[] =
{
    {
        STATE_ID_STARTING, /* toStateId */
        EV_SAVED,          /* eventId */
        0,                 /* flags */
        NULL,              /* guard */
        NULL,              /* action */
        NULL,              /* cookie */
        NULL               /* private: toState */
    }
};


/* Definition of transitions originating from the "Active" state */

static RTHsmTransition gActiveTransitions[] =
{
    {
        STATE_ID_MALFUNCTION, /* toStateId */
        EV_ERROR,             /* eventId */
        0,                    /* flags */
        NULL,                 /* guard */
        NULL,                 /* action */
        NULL,                 /* cookie */
        NULL                  /* private: toState */
    }
};


/* Definition of transitions originating from the "Reading" state */

static void rthsmTestReadingToProcessingAction(const RTHsmEvent* event,
        void* cookie)
{
    (void)event; /* unused argument */

    RTASSERT(cookie == (void*)0xDeadBeef);
    gProcessCount = 0;
}

static void rthsmTestReadingToReadingAction(const RTHsmEvent* event,
        void* cookie)
{
    RTBool pushed;
    RTHsmEvent newEvent;
    static uint8_t counter = 2u;

    (void)event; /* unused argument */
    (void)cookie; /* unused argument */

    counter++;
    if (0 == (counter % 7u)) {
        newEvent.id = EV_ACQUIRED;
    } else {
        newEvent.id = EV_DATA;
    }
    pushed = RTHsmPushEvent(&gHsm, &newEvent);
    RTASSERT(pushed);
}

static RTHsmTransition gReadingTransitions[] =
{
    {
        STATE_ID_PROCESSING,                /* toStateId */
        EV_ACQUIRED,                        /* eventId */
        0,                                  /* flags */
        NULL,                               /* guard */
        rthsmTestReadingToProcessingAction, /* action */
        (void*)0xDeadBeef,                  /* cookie */
        NULL                                /* private: toState */
    },
    {
        STATE_ID_READING,                /* toStateId */
        EV_DATA,                         /* eventId */
        RTHSM_TRANSITION_FLAG_INTERNAL,  /* flags */
        NULL,                            /* guard */
        rthsmTestReadingToReadingAction, /* action */
        NULL,                            /* cookie */
        NULL                             /* private: toState */
    }
};


/* Definition of transitions originating from the "Processing" state */

static void rthsmTestProcessingToProcessingAction(const RTHsmEvent* event,
        void* cookie)
{
    (void)event; /* unused argument */
    (void)cookie; /* unused argument */
    gProcessCount++;
}

static RTHsmTransition gProcessingTransitions[] =
{
    {
        STATE_ID_PROCESSING,                   /* toStateId */
        EV_PROCESSING,                         /* eventId */
        0,                                     /* flags */
        NULL,                                  /* guard */
        rthsmTestProcessingToProcessingAction, /* action */
        NULL,                                  /* cookie */
        NULL                                   /* private: toState */
    },
    {
        STATE_ID_SAVING, /* toStateId */
        EV_PROCESSED,    /* eventId */
        0,               /* flags */
        NULL,            /* guard */
        NULL,            /* action */
        NULL,            /* cookie */
        NULL             /* private: toState */
    }
};


/* Definition of the transitions originating from the "Error" state */

static RTHsmTransition gErrorTransitions[] =
{
    {
        STATE_ID_FINISHED, /* toStateId */
        EV_DEAD,           /* eventId */
        0,                 /* flags */
        NULL,              /* guard */
        NULL,              /* action */
        NULL,              /* cookie */
        NULL               /* private: toState */
    }
};


/* Definition of transitions originating from the "Malfunction" state */

static uint8_t rthsmTestMalfunctionToErrorGuard(const RTHsmEvent* event,
        void* cookie)
{
    (void)event; /* unused argument */
    (void)cookie; /* unused argument */
    return gMalfunctionGuard;
}

static void rthsmTestMalfunctionToErrorAction(const RTHsmEvent* event,
        void* cookie)
{
    RTHsmEvent newEvent;
    RTBool pushed;

    (void)event; /* unused argument */
    (void)cookie; /* unused argument */

    newEvent.id = EV_RECOVER;
    pushed = RTHsmPushEvent(&gHsm, &newEvent);
    RTASSERT(pushed);
}

static RTHsmTransition gMalfunctionTransitions[] =
{
    {
        STATE_ID_ERROR,                    /* toStateId */
        EV_NEXT,                           /* eventId */
        0,                                 /* flags */
        rthsmTestMalfunctionToErrorGuard,  /* guard */
        rthsmTestMalfunctionToErrorAction, /* action */
        NULL,                              /* cookie */
        NULL                               /* private: toState */
    },
    {
        STATE_ID_READING, /* toStateId */
        EV_RECOVER,       /* eventId */
        0,                /* flags */
        NULL,             /* guard */
        NULL,             /* action */
        NULL,             /* cookie */
        NULL              /* private: toState */
    }
};


/* Definition of state actions */

static void rthsmTestDeviceOnExitAction(void* cookie)
{
    (void)cookie; /* unused argument */
    gIteration++;
}

static void rthsmTestReadingEntryAction(void* cookie)
{
    RTHsmEvent event;
    RTBool pushed;

    RTASSERT(cookie == (void*)0x12345678);

    event.id = EV_DATA;
    pushed = RTHsmPushEvent(&gHsm, &event);
    RTASSERT(pushed);
}

static void rthsmTestProcessingEntryAction(void* cookie)
{
    RTHsmEvent event;
    RTBool pushed;

    (void)cookie; /* unused argument */

    if ((gIteration == 2) && (gProcessCount > 5)) {
        event.id = EV_ERROR;
    } else {
        event.id = EV_PROCESSING;
    }
    pushed = RTHsmPushEvent(&gHsm, &event);
    RTASSERT(pushed);
}

static void rthsmTestProcessingExitAction(void* cookie)
{
    (void)cookie; /* unused argument */

    if (gProcessCount > 10) {
        RTHsmEvent event;
        RTBool pushed;
        event.id = EV_PROCESSED;
        pushed = RTHsmPushEvent(&gHsm, &event);
        RTASSERT(pushed);
    }
}


/* Definition of states */

static RTHsmState gStates[] =
{
    {
        STATE_ID_GLOBAL,     /* id */
        0,                   /* flags */
        RTHSM_NULL_STATE_ID, /* parentId */
        STATE_ID_STARTING,   /* initialId */
        NULL,                /* entryAction */
        NULL,                /* exitAction */
        NULL,                /* cookie */
        NULL,                /* transitions */
        0,                   /* transitionsSize */
        NULL,                /* private: parent */
        NULL                 /* private: initial */
    },
    {
        STATE_ID_STARTING,                 /* id */
        0,                                 /* flags */
        STATE_ID_GLOBAL,                   /* parentId */
        RTHSM_NULL_STATE_ID,               /* initialId */
        NULL,                              /* entryAction */
        NULL,                              /* exitAction */
        NULL,                              /* cookie */
        gStartingTransitions,              /* transitions */
        RTARRAYSIZE(gStartingTransitions), /* transitionsSize */
        NULL,                              /* private: parent */
        NULL                               /* private: initial */
    },
    {
        STATE_ID_FINISHED,      /* id */
        RTHSM_STATE_FLAG_FINAL, /* flags */
        STATE_ID_GLOBAL,        /* parentId */
        RTHSM_NULL_STATE_ID,    /* initialId */
        NULL,                   /* entryAction */
        NULL,                   /* exitAction */
        NULL,                   /* cookie */
        NULL,                   /* transitions */
        0,                      /* transitionsSize */
        NULL,                   /* private: parent */
        NULL                    /* private: initial */
    },
    {
        STATE_ID_DEVICE_ON,                /* id */
        0,                                 /* flags */
        STATE_ID_GLOBAL,                   /* parentId */
        STATE_ID_ACTIVE,                   /* initialId */
        NULL,                              /* entryAction */
        rthsmTestDeviceOnExitAction,       /* exitAction */
        NULL,                              /* cookie */
        gDeviceOnTransitions,              /* transitions */
        RTARRAYSIZE(gDeviceOnTransitions), /* transitionsSize */
        NULL,                              /* private: parent */
        NULL                               /* private: initial */
    },
    {
        STATE_ID_ACTIVE,                 /* id */
        0,                               /* flags */
        STATE_ID_DEVICE_ON,              /* parentId */
        STATE_ID_READING,                /* initialId */
        NULL,                            /* entryAction */
        NULL,                            /* exitAction */
        NULL,                            /* cookie */
        gActiveTransitions,              /* transitions */
        RTARRAYSIZE(gActiveTransitions), /* transitionsSize */
        NULL,                            /* private: parent */
        NULL                             /* private: initial */
    },
    {
        STATE_ID_READING,                 /* id */
        0,                                /* flags */
        STATE_ID_ACTIVE,                  /* parentId */
        RTHSM_NULL_STATE_ID,              /* initialId */
        rthsmTestReadingEntryAction,      /* entryAction */
        NULL,                             /* exitAction */
        (void*)0x12345678,                /* cookie */
        gReadingTransitions,              /* transitions */
        RTARRAYSIZE(gReadingTransitions), /* transitionsSize */
        NULL,                             /* private: parent */
        NULL                              /* private: initial */
    },
    {
        STATE_ID_PROCESSING,                 /* id */
        0,                                   /* flags */
        STATE_ID_ACTIVE,                     /* parentId */
        RTHSM_NULL_STATE_ID,                 /* initialId */
        rthsmTestProcessingEntryAction,      /* entryAction */
        rthsmTestProcessingExitAction,       /* exitAction */
        NULL,                                /* cookie */
        gProcessingTransitions,              /* transitions */
        RTARRAYSIZE(gProcessingTransitions), /* transitionsSize */
        NULL,                                /* private: parent */
        NULL                                 /* private: initial */
    },
    {
        STATE_ID_SAVING,     /* id */
        0,                   /* flags */
        STATE_ID_DEVICE_ON,  /* parentId */
        RTHSM_NULL_STATE_ID, /* initialId */
        NULL,                /* entryAction */
        NULL,                /* exitAction */
        NULL,                /* cookie */
        NULL,                /* transitions */
        0,                   /* transitionsSize */
        NULL,                /* private: parent */
        NULL                 /* private: initial */
    },
    {
        STATE_ID_ERROR,                 /* id */
        0,                              /* flags */
        STATE_ID_GLOBAL,                /* parentId */
        STATE_ID_MALFUNCTION,           /* initialId */
        NULL,                           /* entryAction */
        NULL,                           /* exitAction */
        NULL,                           /* cookie */
        gErrorTransitions,              /* transitions */
        RTARRAYSIZE(gErrorTransitions), /* transitionsSize */
        NULL,                           /* private: parent */
        NULL                            /* private: initial */
    },
    {
        STATE_ID_MALFUNCTION,                 /* id */
        0,                                    /* flags */
        STATE_ID_ERROR,                       /* parentId */
        RTHSM_NULL_STATE_ID,                  /* initialId */
        NULL,                                 /* entryAction */
        NULL,                                 /* exitAction */
        NULL,                                 /* cookie */
        gMalfunctionTransitions,              /* transitions */
        RTARRAYSIZE(gMalfunctionTransitions), /* transitionsSize */
        NULL,                                 /* private: parent */
        NULL                                  /* private: initial */
    }
};



/* --- Unit tests --- */


RTT_GROUP_START(HsmInit, 0x00030001u, NULL, NULL)

RTT_TEST_START(hsm_should_initialise)
{
    RTHsmInit(&gHsm, gStates, RTARRAYSIZE(gStates), &gEventQueue);
    RTT_ASSERT(gHsm.current == NULL);
}
RTT_TEST_END

RTT_GROUP_END(HsmInit,
        hsm_should_initialise)


RTT_GROUP_START(HsmRunStateMachine, 0x00030002u, NULL, NULL)


/* First iteration */

RTT_TEST_START(hsm_iter1_should_get_out_of_initial_pseudo_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_STARTING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_push_1st_event)
{
    RTHsmEvent event;
    event.id = EV_NEXT;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_step_to_reading_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
    RTT_ASSERT(gIteration == 0);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_loop_5_steps_in_reading_state)
{
    int8_t i;
    for (i = 0; i < 5; i++) {
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_step_to_processing_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
    RTT_ASSERT(gProcessCount == 0);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_loop_12_steps_in_processing_state)
{
    int8_t i;
    for (i = 0; i < 12; i++) {
        RTT_ASSERT(gProcessCount == i);
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_step_to_saving_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_SAVING);
}
RTT_TEST_END

/* NB: An `EV_PROCESSING` and an `EV_PROCESSED` events should have been left in
 * the event queue from the previous test case.
 */
RTT_TEST_START(hsm_iter1_should_discard_useless_events)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_do_nothing_if_no_event)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_EMPTY);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter1_should_step_to_starting_state)
{
    RTHsmEvent event;
    event.id = EV_SAVED;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_STARTING);
}
RTT_TEST_END


/* Second iteration */

RTT_TEST_START(hsm_iter2_should_step_to_reading_state)
{
    RTHsmEvent event;
    event.id = EV_NEXT;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter2_should_loop_7_steps_in_reading_state)
{
    int8_t i;
    for (i = 0; i < 7; i++) {
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter2_should_step_to_processing_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
    RTT_ASSERT(gProcessCount == 0);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter2_should_loop_12_steps_in_processing_state)
{
    int8_t i;
    for (i = 0; i < 12; i++) {
        RTT_ASSERT(gProcessCount == i);
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter2_should_step_to_saving_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_SAVING);
}
RTT_TEST_END

/* NB: An `EV_PROCESSING` and an `EV_PROCESSED` events should have been left in
 * the event queue from the previous test case.
 */
RTT_TEST_START(hsm_iter2_should_discard_useless_events)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter2_should_do_nothing_if_no_event)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_EMPTY);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter2_should_step_to_starting_state)
{
    RTHsmEvent event;
    event.id = EV_SAVED;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_STARTING);
}
RTT_TEST_END


/* Third iteration */

RTT_TEST_START(hsm_iter3_should_step_to_reading_state)
{
    RTHsmEvent event;
    event.id = EV_NEXT;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter3_should_loop_7_steps_in_reading_state)
{
    int8_t i;
    for (i = 0; i < 7; i++) {
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter3_should_step_to_processing_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
    RTT_ASSERT(gProcessCount == 0);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter3_should_loop_6_steps_in_processing_state)
{
    int8_t i;
    for (i = 0; i < 6; i++) {
        RTT_ASSERT(gProcessCount == i);
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter3_should_step_to_malfunction_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_MALFUNCTION);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter3_guard_should_deny_transition)
{
    uint8_t guardResult;
    RTHsmEvent event;
    event.id = EV_NEXT;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    gMalfunctionGuard = 234u;
    RTT_ASSERT(RTHsmStep(&gHsm, &guardResult) == RTHSM_STEP_RESULT_GUARD);
    RTT_ASSERT(guardResult == 234u);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_MALFUNCTION);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter3_should_step_to_malfunction_state_through_error_state)
{
    RTHsmEvent event;
    gMalfunctionGuard = 0;
    event.id = EV_NEXT;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_MALFUNCTION);
}
RTT_TEST_END


/* Fourth iteration */

RTT_TEST_START(hsm_iter4_should_step_to_reading_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter4_should_loop_7_steps_in_reading_state)
{
    int8_t i;
    for (i = 0; i < 7; i++) {
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter4_should_step_to_processing_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
    RTT_ASSERT(gProcessCount == 0);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter4_should_loop_12_steps_in_processing_state)
{
    int8_t i;
    for (i = 0; i < 12; i++) {
        RTT_ASSERT(gProcessCount == i);
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter4_should_step_to_saving_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_SAVING);
}
RTT_TEST_END

/* NB: An `EV_PROCESSING` and an `EV_PROCESSED` events should have been left in
 * the message queue from the previous test case.
 */
RTT_TEST_START(hsm_iter4_should_discard_useless_events)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter4_should_do_nothing_if_no_event)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_EMPTY);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter4_should_step_to_starting_state)
{
    RTHsmEvent event;
    event.id = EV_SAVED;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_STARTING);
}
RTT_TEST_END


/* Fifth iteration */

RTT_TEST_START(hsm_iter5_should_step_to_reading_state)
{
    RTHsmEvent event;
    event.id = EV_NEXT;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter5_should_loop_7_steps_in_reading_state)
{
    int8_t i;
    for (i = 0; i < 7; i++) {
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_READING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter5_should_step_to_processing_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
    RTT_ASSERT(gProcessCount == 0);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter5_should_loop_12_steps_in_processing_state)
{
    int8_t i;
    for (i = 0; i < 12; i++) {
        RTT_ASSERT(gProcessCount == i);
        RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    }
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_PROCESSING);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter5_should_step_to_saving_state)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_SAVING);
}
RTT_TEST_END

/* NB: An `EV_PROCESSING` and an `EV_PROCESSED` events should have been left in
 * the message queue from the previous test case.
 */
RTT_TEST_START(hsm_iter5_should_discard_useless_events)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_DISCARDED);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter5_should_do_nothing_if_no_event)
{
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_EMPTY);
}
RTT_TEST_END

RTT_TEST_START(hsm_iter5_should_step_to_starting_state)
{
    RTHsmEvent event;
    event.id = EV_SAVED;
    RTT_ASSERT(RTHsmPushEvent(&gHsm, &event));
    RTT_ASSERT(RTHsmStep(&gHsm, NULL) == RTHSM_STEP_RESULT_OK);
    RTT_ASSERT(gHsm.current != NULL);
    RTT_ASSERT(gHsm.current->id == STATE_ID_STARTING);
}
RTT_TEST_END


RTT_GROUP_END(HsmRunStateMachine,
        hsm_iter1_should_get_out_of_initial_pseudo_state,
        hsm_iter1_should_push_1st_event,
        hsm_iter1_should_step_to_reading_state,
        hsm_iter1_should_loop_5_steps_in_reading_state,
        hsm_iter1_should_step_to_processing_state,
        hsm_iter1_should_loop_12_steps_in_processing_state,
        hsm_iter1_should_step_to_saving_state,
        hsm_iter1_should_discard_useless_events,
        hsm_iter1_should_do_nothing_if_no_event,
        hsm_iter1_should_step_to_starting_state,

        hsm_iter2_should_step_to_reading_state,
        hsm_iter2_should_loop_7_steps_in_reading_state,
        hsm_iter2_should_step_to_processing_state,
        hsm_iter2_should_loop_12_steps_in_processing_state,
        hsm_iter2_should_step_to_saving_state,
        hsm_iter2_should_discard_useless_events,
        hsm_iter2_should_do_nothing_if_no_event,
        hsm_iter2_should_step_to_starting_state,

        hsm_iter3_should_step_to_reading_state,
        hsm_iter3_should_loop_7_steps_in_reading_state,
        hsm_iter3_should_step_to_processing_state,
        hsm_iter3_should_loop_6_steps_in_processing_state,
        hsm_iter3_should_step_to_malfunction_state,
        hsm_iter3_guard_should_deny_transition,
        hsm_iter3_should_step_to_malfunction_state_through_error_state,

        hsm_iter4_should_step_to_reading_state,
        hsm_iter4_should_loop_7_steps_in_reading_state,
        hsm_iter4_should_step_to_processing_state,
        hsm_iter4_should_loop_12_steps_in_processing_state,
        hsm_iter4_should_step_to_saving_state,
        hsm_iter4_should_discard_useless_events,
        hsm_iter4_should_do_nothing_if_no_event,
        hsm_iter4_should_step_to_starting_state,

        hsm_iter5_should_step_to_reading_state,
        hsm_iter5_should_loop_7_steps_in_reading_state,
        hsm_iter5_should_step_to_processing_state,
        hsm_iter5_should_loop_12_steps_in_processing_state,
        hsm_iter5_should_step_to_saving_state,
        hsm_iter5_should_discard_useless_events,
        hsm_iter5_should_do_nothing_if_no_event,
        hsm_iter5_should_step_to_starting_state)
