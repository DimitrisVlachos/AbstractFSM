/*
    Abstract Finite state machine implementation (C) 2018 Dimitris Vlachos
    Dimitrisv22@gmail.com
*/

#ifndef _abstract_fsm_machine_hpp_
#define _abstract_fsm_machine_hpp_

template <const bool k_thread_safe>
class state_machine_impl_c {
private:
    std::unordered_map<std::string,abstract_state_if*> m_transitions;
    void cleanup() {
        for (auto i = m_transitions.begin(); i != m_transitions.end(); ++i)
            delete i->second;

        m_transitions.clear();
        m_curr_state = nullptr;
    }
    abstract_state_if* m_curr_state;
    std::recursive_mutex m_mtx;

    inline void lock() {
        if (!k_thread_safe) return;
        while (!m_mtx.try_lock())
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    inline void unlock() {
        if (!k_thread_safe)return;
        m_mtx.unlock();
    }

public:
    state_machine_impl_c() : m_curr_state(nullptr) {
    }
    ~state_machine_impl_c() {
        cleanup();
    }

    bool register_state(const std::string& key,abstract_state_if* state) {
        lock();
        m_transitions.insert( {key,state} );
        unlock();
        return true;
    }

    bool set_state(const std::string& key) {
        lock();
        auto i = m_transitions.find(key);
        if (i == m_transitions.end()) {
            unlock();
            return false;
        }

        m_curr_state = i->second;
        m_curr_state->enter();
        unlock();
        return true;
    }

    bool update_cycle(const uint64_t time_ticks = 0U) {
        lock();
        if (m_curr_state==nullptr) {
            unlock();
            return true;
        }  else if (m_curr_state->is_exit_func()) {
			m_curr_state->update(time_ticks);
            m_curr_state->leave();
            m_curr_state=nullptr;
            unlock();
            return true;
        } else if (m_curr_state->update(time_ticks)) {
            m_curr_state->leave();
            this->set_state( m_curr_state->get_transition_function() );
        }
        unlock();
        return false;
    }
};

#endif

