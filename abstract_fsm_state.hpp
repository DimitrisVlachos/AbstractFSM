/*
    Abstract Finite state machine implementation (C) 2018 Dimitris Vlachos
    Dimitrisv22@gmail.com
*/

#ifndef _abstract_fsm_state_hpp_
#define _abstract_fsm_state_hpp_

class abstract_state_if {
private:
    struct ptr_t {
        void* ptr;
        void (*cleanup_func)(void*);
        ptr_t(void* p,void (*cfun)(void*) = nullptr) : ptr(p),cleanup_func(cfun) {}
        ~ptr_t() {
            if (cleanup_func) {
                cleanup_func(ptr);
            }
        }
    };
    std::unordered_map<std::string,ptr_t> m_user_data;

public:
    abstract_state_if() { }
    virtual ~abstract_state_if() { }
    virtual bool enter() { return false; }
    virtual bool leave() { return false;}
    virtual bool update(const uint64_t time_ticks) { return false; }
    virtual const std::string& get_transition_function() const {
        static const std::string k_null_trans_func = "null";
        return k_null_trans_func;
    }
    virtual const bool is_exit_func() const { return true; }


    inline bool add_user_data(const std::string& key,void* data,void (*my_cleanup_func)(void*) = nullptr) {
        if (get_user_data(key) != nullptr)
            return false;
        m_user_data.insert( {key, ptr_t(data,my_cleanup_func) } );
        return true;
    }

    inline void* get_user_data(const std::string& key) {
        auto it = m_user_data.find(key);
        if (it == m_user_data.end())
            return nullptr;
        return static_cast<void*>(it->second.ptr);
    }
};

#endif

