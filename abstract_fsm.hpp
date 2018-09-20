/*
    Abstract Finite state machine implementation (C) 2018 Dimitris Vlachos
    Dimitrisv22@gmail.com
*/
#ifndef __abstract_fsm_hpp__
#define __abstract_fsm_hpp__
#include "abstract_fsm_includes.hpp"
#include "abstract_fsm_state.hpp"
#include "abstract_fsm_machine.hpp"
#include "abstract_fsm_complex_machine.hpp"

typedef state_machine_impl_c<true> abstract_state_machine_thread_safe_c;
typedef state_machine_impl_c<false> abstract_state_machine_c;
typedef complex_state_machine_impl_c<true> complex_state_machine_thread_safe_c;
typedef complex_state_machine_impl_c<false> complex_state_machine;

#endif

