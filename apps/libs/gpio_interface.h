/*
 * gpio_interface.h
 */

#ifndef __GPIO_INTERFACE_H
#define __GPIO_INTERFACE_H

#include "mcp23017.h"

#ifdef __cplusplus
extern "C" {
#endif
void config_gpio(int gpio);
void write_gpio(int gpio, bool val);
bool read_gpio(int gpio);
#ifdef __cplusplus
};
#endif

typedef enum {
    GPIO_0 = 0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    GPIO_15,
    GPIO_16,
    GPIO_17,
    GPIO_18,
    GPIO_MAX
} t_gpio_pin;

/*
 * GPIO mapping
 *
 * GP0   GPB0 - MCP23017
 * GP1   GPB1 - MCP23017
 * ...
 * GP7   GPB7 - MCP23017
 * GP8   GPA0 - MCP23017
 * GP9   GPA1 - MCP23017
 * ...
 * GP15  GPA7 - MCP23017
 * GP16  PC1
 * GP17  PB0
 * GP18  PC0
 */

#define IS_PORT_A(p)    (p >=8 && p <= 15)
#define IS_PORT_B(p)    (p >=0 && p <= 7)
#define IS_GPIO_PIN(p)  (p >=16 && p <= 18)

#define TO_PORT_A(p)    (p - 8)
#define TO_PORT_B(p)    (p)
#define TO_GPIO_PIN(p)  (p - 16)

class GPIO_Dev {
 public:
    GPIO_Dev(MCP23017_Dev * expander);
    bool init();
    void set_dir(t_gpio_pin pin, GPIO_dir dir);
    void set_input(t_gpio_pin pin) { set_dir(pin, gpio_IN); };
    void set_output(t_gpio_pin pin) { set_dir(pin, gpio_OUT); };
    bool get_pin(t_gpio_pin pin);
    void set_pin(t_gpio_pin pin, bool val);
    bool event(t_gpio_pin pin);
    void poll() { m_expander->poll(); };
 private:
    MCP23017_Dev * m_expander;
    bool m_state[GPIO_MAX];
    GPIO_dir m_dir[GPIO_MAX];
};

extern GPIO_Dev * gpio;

namespace GPIO {
    inline void set_input(t_gpio_pin pin) { gpio->set_input(pin); };
    inline void set_output(t_gpio_pin pin) { gpio->set_output(pin); };
    inline bool get_pin(t_gpio_pin pin) { return gpio->get_pin(pin); };
    inline void set_pin(t_gpio_pin pin, bool val) { gpio->set_pin(pin, val); };
};

#endif

