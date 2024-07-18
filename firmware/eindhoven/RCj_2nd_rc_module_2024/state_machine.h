#ifndef STATE_H
#define STATE_H

// BLE Massage IDs
enum stm_states {
    STM_INIT = 0,
    STM_DISCONNECTED,
    STM_PLAY,
    STM_STOP,
    STM_DAMAGE,
    STM_HALF_TIME,
    STM_GAME_OVER,
    STM_UPDATING,
    
}; 

int8_t stm_init();

int8_t stm_update();

void stm_set_state(stm_states state);

int8_t stm_set_timer(uint32_t miliseconds);



#endif // STATE_H
