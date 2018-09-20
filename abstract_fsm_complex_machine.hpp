/*
    Abstract Finite state machine implementation (C) 2018 Dimitris Vlachos
    Dimitrisv22@gmail.com
*/

#ifndef _abstract_fsm_complex_machine_hpp_
#define _abstract_fsm_complex_machine_hpp_

template <const bool k_thread_safe>
class complex_state_machine_impl_c {
private:
	struct complex_state_ctx_t {
		bool completed;
		std::vector<abstract_state_if*> nodes;
		std::string transition_block;
		complex_state_ctx_t() : completed(false) {}
		complex_state_ctx_t(const std::vector<abstract_state_if*>& v,const std::string& t,const bool completed_) : nodes(v),transition_block(t),completed(completed_) {}
		complex_state_ctx_t(std::initializer_list<abstract_state_if*> v,const std::string& t,const bool completed_) : nodes(v),transition_block(t),completed(completed_) {}
	};

    std::unordered_map<std::string,complex_state_ctx_t> m_transitions;
    void cleanup() {
        for (auto i = m_transitions.begin(); i != m_transitions.end(); ++i) {
			for (auto ptr : i->second.nodes)
           		delete ptr;
		}

        m_transitions.clear();
        m_curr_state = nullptr;
    }
    complex_state_ctx_t* m_curr_state;
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

	inline bool do_update(const uint64_t ticks) {
		if (m_curr_state->nodes.empty())
			return true;
		
		bool all_completed = true;

		for (auto i = 0,j = m_curr_state->nodes.size();i < j;++i) {
			auto ptr = m_curr_state->nodes[i];
			if (ptr->completed)
				continue;
			else if (ptr->is_exit_func()) {
				ptr->leave();
				ptr->completed = true;
				m_curr_state = nullptr;
				continue;
			}
			else if (ptr->update(ticks)) {
				ptr->leave();
				ptr->completed = true;
			} else all_completed = false;
		}
		return all_completed;
	}
public:
    complex_state_machine_impl_c() : m_curr_state(nullptr) {
    }
    ~complex_state_machine_impl_c() {
        cleanup();
    }

	void reset() {	
        for (auto i = m_transitions.begin(); i != m_transitions.end(); ++i) {
			for (auto v : i->second)
           		v.completed = false;
		}
	}

    bool register_state(const std::string& key,const std::string& transition_block_key,const std::vector<abstract_state_if*>& state_block) {
        lock();
        m_transitions.insert( {key,{state_block,transition_block_key,false} } );
        unlock();
        return true;
    }

    bool register_state(const std::string& key,const std::string& transition_block_key,std::initializer_list<abstract_state_if*> state_block_ilist) {
        lock();
        m_transitions.insert( {key,{state_block_ilist,transition_block_key,false} } );
        unlock();
        return true;
    }

    bool set_state(const std::string& key,const bool full_reset) {
        lock();
		if (full_reset)
			reset();

        auto i = m_transitions.find(key);
        if (i == m_transitions.end()) {
            unlock();
            return false;
        }

        m_curr_state = &i->second;
		for (auto ptr : m_curr_state)
        	ptr->enter();

        unlock();
        return true;
    }

    bool update_cycle(const uint64_t time_ticks = 0U) {
        lock();
        if (m_curr_state==nullptr) {
            unlock();
            return true;
        } else if (do_update(time_ticks)) {
            this->set_state( m_curr_state->transition_block );
        	unlock();
      	  	return true;
        }

        unlock();
        return false;
    }
};

#endif

