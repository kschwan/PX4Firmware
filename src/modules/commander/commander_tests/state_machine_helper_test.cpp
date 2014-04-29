/****************************************************************************
 *
 *   Copyright (C) 2013 PX4 Development Team. All rights reserved.
 *   Author: Simon Wilks <sjwilks@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file state_machine_helper_test.cpp
 * System state machine unit test.
 *
 */

#include "state_machine_helper_test.h"

#include "../state_machine_helper.h"
#include <unit_test/unit_test.h>

class StateMachineHelperTest : public UnitTest
{
public:
	StateMachineHelperTest();
	virtual ~StateMachineHelperTest();

	virtual void runTests(void);

private:
	bool armingStateTransitionTest();
	bool mainStateTransitionTest();
	bool isSafeTest();
};

StateMachineHelperTest::StateMachineHelperTest() {
}

StateMachineHelperTest::~StateMachineHelperTest() {
}

bool StateMachineHelperTest::armingStateTransitionTest(void)
{
    // These are the critical values from vehicle_status_s and actuator_armed_s which must be primed
    // to simulate machine state prior to testing an arming state transition. This structure is also
    // use to represent the expected machine state after the transition has been requested.
    typedef struct {
        arming_state_t  arming_state;   // vehicle_status_s.arming_state
        bool            armed;          // actuator_armed_s.armed
        bool            ready_to_arm;   // actuator_armed_s.ready_to_arm
    } ArmingTransitionVolatileState_t;
    
    // This structure represents a test case for arming_state_transition. It contains the machine
    // state prior to transtion, the requested state to transition to and finally the expected
    // machine state after transition.
    typedef struct {
        const char*                     assertMsg;                              // Text to show when test case fails
        ArmingTransitionVolatileState_t current_state;                          // Machine state prior to transtion
        hil_state_t                     hil_state;                              // Current vehicle_status_s.hil_state
        bool                            condition_system_sensors_initialized;   // Current vehicle_status_s.condition_system_sensors_initialized
        bool                            safety_switch_available;                // Current safety_s.safety_switch_available
        bool                            safety_off;                             // Current safety_s.safety_off
        arming_state_t                  requested_state;                        // Requested arming state to transition to
        ArmingTransitionVolatileState_t expected_state;                         // Expected machine state after transition
        transition_result_t             expected_transition_result;             // Expected result from arming_state_transition
    } ArmingTransitionTest_t;
    
    // We use these defines so that our test cases are more readable
    #define ATT_ARMED true
    #define ATT_DISARMED false
    #define ATT_READY_TO_ARM true
    #define ATT_NOT_READY_TO_ARM false
    #define ATT_SENSORS_INITIALIZED true
    #define ATT_SENSORS_NOT_INITIALIZED false
    #define ATT_SAFETY_AVAILABLE true
    #define ATT_SAFETY_NOT_AVAILABLE true
    #define ATT_SAFETY_OFF true
    #define ATT_SAFETY_ON false
    
    // These are test cases for arming_state_transition
    static const ArmingTransitionTest_t rgArmingTransitionTests[] = {
        // TRANSITION_NOT_CHANGED tests
        
        { "no transition: identical states",
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_INIT,
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_NOT_CHANGED },
        
        // TRANSITION_CHANGED tests
        
        // Check all basic valid transitions, these don't require special state in vehicle_status_t or safety_s
        
        { "transition: init to standby",
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: init to standby error",
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY_ERROR,
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: init to reboot",
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_REBOOT,
            { ARMING_STATE_REBOOT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: standby to init",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_INIT,
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: standby to standby error",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY_ERROR,
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: standby to reboot",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_REBOOT,
            { ARMING_STATE_REBOOT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: armed to standby",
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: armed to armed error",
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED_ERROR,
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: armed error to standby error",
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY_ERROR,
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: standby error to reboot",
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_REBOOT,
            { ARMING_STATE_REBOOT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: in air restore to armed",
            { ARMING_STATE_IN_AIR_RESTORE, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED,
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: in air restore to reboot",
            { ARMING_STATE_IN_AIR_RESTORE, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_REBOOT,
            { ARMING_STATE_REBOOT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        // hil on tests, standby error to standby not normally allowed
        
        { "transition: standby error to standby, hil on",
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_ON, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        // Safety switch arming tests
        
        { "transition: init to standby, no safety switch",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_NOT_AVAILABLE, ATT_SAFETY_OFF,
            ARMING_STATE_ARMED,
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        { "transition: init to standby, safety switch off",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_OFF,
            ARMING_STATE_ARMED,
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        // standby error
        { "transition: armed error to standby error requested standby",
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_CHANGED },
        
        // TRANSITION_DENIED tests
        
        // Check some important basic invalid transitions, these don't require special state in vehicle_status_t or safety_s
        
        { "no transition: init to armed",
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED,
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: standby to armed error",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED_ERROR,
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: armed to init",
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_INIT,
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: armed to reboot",
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_REBOOT,
            { ARMING_STATE_ARMED, ATT_ARMED, ATT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: armed error to armed",
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED,
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: armed error to reboot",
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_REBOOT,
            { ARMING_STATE_ARMED_ERROR, ATT_ARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: standby error to armed",
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED,
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: standby error to standby",
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_STANDBY_ERROR, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: reboot to armed",
            { ARMING_STATE_REBOOT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED,
            { ARMING_STATE_REBOOT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        { "no transition: in air restore to standby",
            { ARMING_STATE_IN_AIR_RESTORE, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_IN_AIR_RESTORE, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        // Sensor tests
        
        { "no transition: init to standby - sensors not initialized",
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_NOT_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_STANDBY,
            { ARMING_STATE_INIT, ATT_DISARMED, ATT_NOT_READY_TO_ARM }, TRANSITION_DENIED },
        
        // Safety switch arming tests
        
        { "no transition: init to standby, safety switch on",
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, HIL_STATE_OFF, ATT_SENSORS_INITIALIZED, ATT_SAFETY_AVAILABLE, ATT_SAFETY_ON,
            ARMING_STATE_ARMED,
            { ARMING_STATE_STANDBY, ATT_DISARMED, ATT_READY_TO_ARM }, TRANSITION_DENIED },
    };
    
	struct vehicle_status_s status;
	struct safety_s         safety;
	struct actuator_armed_s armed;
    
    size_t cArmingTransitionTests = sizeof(rgArmingTransitionTests) / sizeof(rgArmingTransitionTests[0]);
    for (size_t i=0; i<cArmingTransitionTests; i++) {
        const ArmingTransitionTest_t* test = &rgArmingTransitionTests[i];
        
        // Setup initial machine state
        status.arming_state = test->current_state.arming_state;
        status.condition_system_sensors_initialized = test->condition_system_sensors_initialized;
        status.hil_state = test->hil_state;
        safety.safety_switch_available = test->safety_switch_available;
        safety.safety_off = test->safety_off;
        armed.armed = test->current_state.armed;
        armed.ready_to_arm = test->current_state.ready_to_arm;
        
        // Attempt transition
        transition_result_t result = arming_state_transition(&status, &safety, test->requested_state, &armed);
        
        // Validate result of transition
        ut_assert(test->assertMsg, test->expected_transition_result == result);
        ut_assert(test->assertMsg, status.arming_state == test->expected_state.arming_state);
        ut_assert(test->assertMsg, armed.armed == test->expected_state.armed);
        ut_assert(test->assertMsg, armed.ready_to_arm == test->expected_state.ready_to_arm);
    }

	return true;
}

bool StateMachineHelperTest::mainStateTransitionTest(void)
{
	struct vehicle_status_s current_state;
	main_state_t new_main_state;
	
	// Identical states.
	current_state.main_state = MAIN_STATE_MANUAL;
	new_main_state = MAIN_STATE_MANUAL;
	ut_assert("no transition: identical states",
		  TRANSITION_NOT_CHANGED == main_state_transition(&current_state, new_main_state));
	ut_assert("current state: manual", MAIN_STATE_MANUAL == current_state.main_state);	

	// AUTO to MANUAL.
	current_state.main_state = MAIN_STATE_AUTO;
	new_main_state = MAIN_STATE_MANUAL;
	ut_assert("transition changed: auto to manual",
		  TRANSITION_CHANGED == main_state_transition(&current_state, new_main_state));
	ut_assert("new state: manual", MAIN_STATE_MANUAL == current_state.main_state);

	// MANUAL to SEATBELT.
	current_state.main_state = MAIN_STATE_MANUAL;
	current_state.condition_local_altitude_valid = true;
	new_main_state = MAIN_STATE_SEATBELT;
	ut_assert("tranisition: manual to seatbelt", 
		  TRANSITION_CHANGED == main_state_transition(&current_state, new_main_state));
	ut_assert("new state: seatbelt", MAIN_STATE_SEATBELT == current_state.main_state);

	// MANUAL to SEATBELT, invalid local altitude.
	current_state.main_state = MAIN_STATE_MANUAL;
	current_state.condition_local_altitude_valid = false;
	new_main_state = MAIN_STATE_SEATBELT;
	ut_assert("no transition: invalid local altitude",
		  TRANSITION_DENIED == main_state_transition(&current_state, new_main_state));
	ut_assert("current state: manual", MAIN_STATE_MANUAL == current_state.main_state);

	// MANUAL to EASY.
	current_state.main_state = MAIN_STATE_MANUAL;
	current_state.condition_local_position_valid = true;
	new_main_state = MAIN_STATE_EASY;
	ut_assert("transition: manual to easy",
		  TRANSITION_CHANGED == main_state_transition(&current_state, new_main_state));
	ut_assert("current state: easy", MAIN_STATE_EASY == current_state.main_state);

	// MANUAL to EASY, invalid local position.
	current_state.main_state = MAIN_STATE_MANUAL;
	current_state.condition_local_position_valid = false;
	new_main_state = MAIN_STATE_EASY;
	ut_assert("no transition: invalid position",
		  TRANSITION_DENIED == main_state_transition(&current_state, new_main_state));
	ut_assert("current state: manual", MAIN_STATE_MANUAL == current_state.main_state);

	// MANUAL to AUTO.
	current_state.main_state = MAIN_STATE_MANUAL;
	current_state.condition_global_position_valid = true;
	new_main_state = MAIN_STATE_AUTO;
	ut_assert("transition: manual to auto",
		  TRANSITION_CHANGED == main_state_transition(&current_state, new_main_state));
	ut_assert("current state: auto", MAIN_STATE_AUTO == current_state.main_state);

	// MANUAL to AUTO, invalid global position.
	current_state.main_state = MAIN_STATE_MANUAL;
	current_state.condition_global_position_valid = false;
	new_main_state = MAIN_STATE_AUTO;
	ut_assert("no transition: invalid global position",
		  TRANSITION_DENIED == main_state_transition(&current_state, new_main_state));
	ut_assert("current state: manual", MAIN_STATE_MANUAL == current_state.main_state);

	return true;
}

bool StateMachineHelperTest::isSafeTest(void)
{
	struct vehicle_status_s current_state;
	struct safety_s safety;
	struct actuator_armed_s armed;

	armed.armed = false;
	armed.lockdown = false;
	safety.safety_switch_available = true;
	safety.safety_off = false;
	ut_assert("is safe: not armed", is_safe(&current_state, &safety, &armed));

	armed.armed = false;
	armed.lockdown = true;
	safety.safety_switch_available = true;
	safety.safety_off = true;
	ut_assert("is safe: software lockdown", is_safe(&current_state, &safety, &armed));

	armed.armed = true;
	armed.lockdown = false;
	safety.safety_switch_available = true;
	safety.safety_off = true;
	ut_assert("not safe: safety off", !is_safe(&current_state, &safety, &armed));

	armed.armed = true;
	armed.lockdown = false;
	safety.safety_switch_available = true;
	safety.safety_off = false;
	ut_assert("is safe: safety off", is_safe(&current_state, &safety, &armed));

	armed.armed = true;
	armed.lockdown = false;
	safety.safety_switch_available = false;
	safety.safety_off = false;
	ut_assert("not safe: no safety switch", !is_safe(&current_state, &safety, &armed));

	return true;
}

void StateMachineHelperTest::runTests(void)
{
	ut_run_test(armingStateTransitionTest);
	ut_run_test(mainStateTransitionTest);
	ut_run_test(isSafeTest);
}

void stateMachineHelperTest(void)
{
	StateMachineHelperTest* test = new StateMachineHelperTest();
    test->runTests();
	test->printResults();
}
